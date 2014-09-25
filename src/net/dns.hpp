/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_DNS_HPP
# define LIBIGHT_NET_DNS_HPP

//
// DNS client functionality
//

#include "common/log.h"
#include "common/poller.h"
#include "common/utils.h"

#include <event2/dns.h>

#include <arpa/inet.h>

#include <functional>
#include <vector>
#include <string>

namespace ight {

//
// This class represents a DNS response.
//
// The fields are the ones returned by evdns, plus the RTT because Neubot
// may want to measure the time-to-reply of DNS servers.
//
struct DNSResponse {

    int count;
    std::vector<std::string> results;
    int result;
    double rtt;
    int ttl;
    char type;

    DNSResponse(int result, char type, int count, int ttl, double rtt) {
        this->result = result;
        this->count = count;
        this->type = type;
        this->ttl = ttl;
        this->rtt = rtt;
    }
};

//
// The state of a DNS request.
//
// This is the internal object thanks to which DNSRequest is movable. Of
// course, DNSRequestState is not movable, since its address is passed to
// one of the many evnds delayed requests functions.
//
class DNSRequestState {

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
    // cancel pending evdns requests and uses a cancelled bool
    // to keep track of cancelled requests.
    //

    std::function<void(DNSResponse&&)> callback;
    bool cancelled = false;
    bool pending = false;
    double ticks;

    static bool process_ipv46_reply(DNSResponse& response, void *addresses,
                                    int family, int size) {
        int count;
        char string[128];            // Should be enough

        for (count = 0; count < response.count; ++count) {
            if (count > INT_MAX / size) {
                ight_info("dns - integer overflow");
                return (false);
            }
            // Note: address already in network byte order
            if (inet_ntop(family, (char *)addresses + count * size,
                string, sizeof (string)) == NULL) {
                ight_info("dns - inet_ntop failed");
                return (false);
            }
            ight_info("dns - adding '%s'", string);
            response.results.push_back(string);
        }

        return (true);
    }

    bool process_dns_reply(DNSResponse& response, void *addresses) {
        bool retval = false;

        switch (response.type) {
        case DNS_IPv4_A:
            ight_info("dns - IPv4");
            retval = process_ipv46_reply(response, addresses, PF_INET, 4);
            break;
        case DNS_IPv6_AAAA:
            ight_info("dns - IPv6");
            retval = process_ipv46_reply(response, addresses, PF_INET6, 16);
            break;
        case DNS_PTR:
            ight_info("dns - PTR");
            // Note: cast magic copied from libevent regress tests
            response.results.push_back(std::string(
                *(char **) addresses));
            retval = true;
            break;
        default:
            ight_info("dns - invalid type");
            retval = false;    // Yes, this is redundant
            break;
        }

        return (retval);
    }

    static void handle_resolve(int result, char type, int count, int ttl,
                               void *addresses, void *opaque) {
        auto state = static_cast<DNSRequestState *>(opaque);

        if (state->cancelled) {
            delete state;
            return;
        }
        state->pending = false;

        auto rtt = ight_time_now() - state->ticks;
        auto response = DNSResponse(result, type, count, ttl, rtt);
        if (result == DNS_ERR_NONE) {
            if (!state->process_dns_reply(response, addresses)) {
                response.result = DNS_ERR_UNKNOWN;
            }
        }

        state->callback(std::move(response));
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
    DNSRequestState(std::string query, std::string address,
                    std::function<void(DNSResponse&&)> f,
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
        if (cancelled)    // Idempotent
            return;
        cancelled = true;
        if (pending)      // Delayed free
            return;
        delete this;
    }
};

//
// Async DNS request.
//
// This is the toplevel class that you should use to issue async
// DNS requests; it supports A, AAAA and PTR queries.
//
class DNSRequest {
    DNSRequestState *state = nullptr;

  public:
    DNSRequest(std::string query, std::string address,
               std::function<void(DNSResponse&&)> func,
               evdns_base *dnsb = NULL) {
        if (dnsb == NULL) {
            dnsb = ight_get_global_evdns_base();
        }
        state = new DNSRequestState(query, address, func, dnsb);
    }

    DNSRequest(DNSRequest& /*other*/) = delete;
    DNSRequest& operator=(DNSRequest& /*other*/) = delete;
    DNSRequest(DNSRequest&& other) {
        std::swap(state, other.state);
    }
    DNSRequest& operator=(DNSRequest&& other) {
        std::swap(state, other.state);
        return (*this);
    }

    ~DNSRequest(void) {
        if (state == nullptr)
            return;
        state->cancel();
        state = nullptr;    // Idempotent
    }
};

}  // namespace
#endif  // LIBIGHT_NET_DNS_HPP
