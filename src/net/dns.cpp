/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "net/dns.hpp"

#include "common/log.h"
#include "common/utils.h"

#include <event2/dns.h>
#include <arpa/inet.h>

using namespace ight;

DNSResponse::DNSResponse(void) : code(DNS_ERR_UNKNOWN)
{
    // nothing
}

DNSResponse::DNSResponse(int code, char type, int count, int ttl,
                         double rtt, void *addresses)
    : code(code), type(type), ttl(ttl), rtt(rtt)
{
    if (code != DNS_ERR_NONE) {
        ight_info("dns - request failed: %d", code);
        // do not process the results if there was an error

    } else if (type == DNS_PTR) {
        ight_info("dns - PTR");
        // Note: cast magic copied from libevent regress tests
        results.push_back(std::string(*(char **) addresses));

    } else if (type == DNS_IPv4_A || type == DNS_IPv6_AAAA) {

        int family;
        int size;
        char string[128];  // Should be enough

        if (type == DNS_IPv4_A) {
            family = PF_INET;
            size = 4;
            ight_info("dns - IPv4");
        } else {
            family = PF_INET6;
            size = 16;
            ight_info("dns - IPv6");
        }

        for (auto i = 0; i < count; ++i) {
            if (i > INT_MAX / size) {
                ight_warn("Integer overflow");
                code = DNS_ERR_UNKNOWN;
                break;
            }
            // Note: address already in network byte order
            if (inet_ntop(family, (char *)addresses + i * size,
                string, sizeof (string)) == NULL) {
                ight_warn("Unexpected inet_ntop failure");
                code = DNS_ERR_UNKNOWN;
                break;
            }
            ight_info("dns - adding '%s'", string);
            results.push_back(string);
        }

    } else {
        throw std::runtime_error("Invalid response type");
    }
}

namespace ight {

//
// Implementation of DNSRequest.
//
// This is the internal object thanks to which DNSRequest is movable. Of
// course, DNSRequestImpl is not movable, since its address is passed to
// one of the many evnds delayed requests functions.
//
class DNSRequestImpl {

    //
    // Note: evdns_base_resolve_xxx() return a evdns_request
    // object that may be used to cancel the request. Yet, the
    // semantic of cancelling a request is such that evdns
    // could invoke the callback of a pending request if such
    // request was cancelled when its corresponding response
    // was already pending. This seems to be confirmed by
    // comments in libevent's evdns.c:
    //
    //     This does nothing if the request's callback is
    //     already running (pending_cb is set)
    //
    // For the above reason this class does not explicitly
    // cancel pending evdns requests and uses the `cancelled`
    // variable to keep track of cancelled requests.
    //

    std::function<void(DNSResponse&&)> callback;
    bool cancelled = false;
    bool pending = false;
    double ticks;

    static void handle_resolve(int code, char type, int count, int ttl,
                               void *addresses, void *opaque) {

        auto impl = static_cast<DNSRequestImpl *>(opaque);
        auto rtt = ight_time_now() - impl->ticks;

        if (impl->cancelled) {
            delete impl;
            return;
        }
        impl->pending = false;

        impl->callback(DNSResponse(code, type, count, ttl, rtt, addresses));
    }

    in_addr *ipv4_pton(std::string address, in_addr *netaddr) {
        if (inet_pton(AF_INET, address.c_str(), netaddr) != 1)
            throw std::runtime_error("Invalid IPv4 address");
        return (netaddr);
    }

    in6_addr *ipv6_pton(std::string address, in6_addr *netaddr) {
        if (inet_pton(AF_INET6, address.c_str(), netaddr) != 1)
            throw std::runtime_error("Invalid IPv6 address");
        return (netaddr);
    }

  public:
    DNSRequestImpl(std::string query, std::string address,
                   std::function<void(DNSResponse&&)>&& f,
                   evdns_base *base) : callback(f) {

        //
        // We explain above why we don't store the return value
        // of the evdns_base_resolve_xxx() functions below
        //
        if (query == "A") {
            if (evdns_base_resolve_ipv4(base, address.c_str(),
                DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "AAAA") {
            if (evdns_base_resolve_ipv6(base, address.c_str(),
                DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_A") {
            in_addr na;
            if (evdns_base_resolve_reverse(base, ipv4_pton(address,
                &na), DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_AAAA") {
            in6_addr na;
            if (evdns_base_resolve_reverse_ipv6(base, ipv6_pton(address,
                &na), DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else
            throw std::runtime_error("Unsupported query");

        pending = true;
        ticks = ight_time_now();
    }

    void cancel(void) {
        if (cancelled) {    // Idempotent
            return;
        }
        cancelled = true;
        if (pending) {     // Delayed free
            return;
        }
        delete this;
    }
};

}  // namespace

//
// Async DNS request.
//

DNSRequest::DNSRequest(std::string query, std::string address,
                       std::function<void(DNSResponse&&)>&& func,
                       evdns_base *dnsb)
{
    if (dnsb == NULL) {
        dnsb = ight_get_global_evdns_base();
    }
    impl = new DNSRequestImpl(query, address, std::move(func), dnsb);
}

DNSRequest::~DNSRequest(void)
{
    if (impl == nullptr) {
        return;
    }
    impl->cancel();
    impl = nullptr;    // Idempotent
}

//
// DNS Resolver object.
//

void
DNSResolver::cleanup(void)
{
    if (base != NULL) {
        //
        // Note: `1` means that pending requests are notified that
        // this evdns_base is being closed.
        //
        evdns_base_free(base, 1);
        base = NULL;  // Idempotent
    }
}

DNSResolver::DNSResolver(std::string nameserver, std::string attempts,
                         IghtPoller *poller)
{
    if (nameserver == "" && attempts == "" && poller == NULL) {
        // No specific options? Then let's use the default evdns_base
        return;
    }
    if (poller == NULL) {
        poller = ight_get_global_poller();
    }
    auto evb = poller->get_event_base();
    if (nameserver != "") {
        if ((base = evdns_base_new(evb, 0)) == NULL) {
            throw std::bad_alloc();
        }
        if (evdns_base_nameserver_ip_add(base, nameserver.c_str()) != 0) {
            cleanup();
            throw std::runtime_error("Cannot set server address");
        }
    } else if ((base = evdns_base_new(evb, 1)) == NULL) {
        throw std::bad_alloc();
    }
    if (attempts != "" && evdns_base_set_option(base, "attempts",
                          attempts.c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'attempts' option");
    }
}

evdns_base *
DNSResolver::get_evdns_base(void)
{
    if (base == NULL) {
        return ight_get_global_evdns_base();
    }
    return base;
}
