/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "protocols/dns.hpp"

#include "common/log.h"
#include "common/utils.hpp"

#include <event2/dns.h>

#include <cassert>

using namespace ight::protocols::dns;

//
// DNS response.
//

Response::Response(int code_, char type, int count, int ttl_, double started,
                   void *addresses, IghtLibevent *libevent, int start_from)
    : code(code_), ttl(ttl_)
{
    assert(start_from >= 0);

    if (libevent == NULL) {
        libevent = IghtGlobalLibevent::get();
    }

    // Only compute RTT when we know that the server replied
    switch (code) {
    case DNS_ERR_NONE:
    case DNS_ERR_FORMAT:
    case DNS_ERR_SERVERFAILED:
    case DNS_ERR_NOTEXIST:
    case DNS_ERR_NOTIMPL:
    case DNS_ERR_REFUSED:
    case DNS_ERR_TRUNCATED:
    case DNS_ERR_NODATA:
        rtt = ight_time_now() - started;
        break;
    default:
        rtt = 0.0;
        break;
    }

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
        char string[128];  // Is wide enough (max. IPv6 length is 45 chars)

        if (type == DNS_IPv4_A) {
            family = PF_INET;
            size = 4;
            ight_info("dns - IPv4");
        } else {
            family = PF_INET6;
            size = 16;
            ight_info("dns - IPv6");
        }

        //
        // Note: make sure in advance `i * size` won't overflow,
        // this is here only for robustness.
        //
        if (count >= 0 && count <= INT_MAX / size + 1) {

            // Note: `start_from`, required by the unit test, defaults to 0
            for (auto i = start_from; i < count; ++i) {
                // Note: address already in network byte order
                if (libevent->inet_ntop(family, (char *)addresses + i * size,
                            string, sizeof (string)) == NULL) {
                    ight_warn("dns - unexpected inet_ntop failure");
                    code = DNS_ERR_UNKNOWN;
                    break;
                }
                ight_info("dns - adding '%s'", string);
                results.push_back(string);
            }

        } else {
            ight_warn("dns - too many addresses");
            code = DNS_ERR_UNKNOWN;
        }

    } else {
        ight_warn("dns - invalid response type");
        code = DNS_ERR_UNKNOWN;
    }
}

std::string
Response::map_failure_(int code)
{
    std::string s;

    //
    // Here we map evdns error codes to OONI failures.
    //
    // We start with errors specified in RFC 1035 (see also event2/dns.h).
    //
    // TODO for @hellais: make sure that the mapping below is
    // correct with respect to OONI, and eventually consider
    // the possiblity of specifying more errors.
    //

    if (code == DNS_ERR_NONE) {
        // nothing to do

    } else if (code == DNS_ERR_FORMAT) {
        // The name server was unable to interpret the query
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_SERVERFAILED) {
        // The name server was unable to process this query due to a
        // problem with the name server
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_NOTEXIST) {
        // The domain name does not exist
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_NOTIMPL) {
        // The name server does not support the requested kind of query
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_REFUSED) {
        // The name server refuses to perform the specified operation
        // for policy reasons
        s += "dns_lookup_error";

    //
    // The following are evdns specific errors
    //

    } else if (code == DNS_ERR_TRUNCATED) {
        // The reply was truncated or ill-formatted
        s += "dns_lookup_error";

    } else if (code == DNS_ERR_UNKNOWN) {
        // An unknown error occurred
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_TIMEOUT) {
        // Communication with the server timed out
        s += "deferred_timeout_error";

    } else if (code == DNS_ERR_SHUTDOWN) {
        // The request was canceled because the DNS subsystem was shut down.
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_CANCEL) {
        // The request was canceled via a call to evdns_cancel_request
        s += "unknown failure ";
        s += std::to_string(code);

    } else if (code == DNS_ERR_NODATA) {
        // There were no answers and no error condition in the DNS packet.
        // This can happen when you ask for an address that exists, but
        // a record type that doesn't.
        s += "dns_lookup_error";

    } else {
        // Safery net - should really not happen
        s += "unknown failure ";
        s += std::to_string(code);
    }

    return s;
}

namespace ight {
namespace protocols {
namespace dns {

/*!
 * \brief Implementation of Request.
 *
 * This is the internal object thanks to which Request is movable. Of
 * course, RequestImpl is not movable, since its address is passed to
 * one of the many evnds delayed requests functions.
 */
class RequestImpl {

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

    std::function<void(Response&&)> callback;
    double ticks = 0.0;  // just to initialize to something
    IghtLibevent *libevent;  // should not be NULL (this is asserted below)
    SharedPointer<bool> cancelled;

    static void handle_resolve(int code, char type, int count, int ttl,
                               void *addresses, void *opaque) {

        auto impl = static_cast<RequestImpl *>(opaque);

        // Tell the libevent layer we received a DNS response
        if (impl->libevent->evdns_reply_hook) {
            impl->libevent->evdns_reply_hook(code, type, count, ttl,
                                             addresses, opaque);
        }

        // Note: the case of `impl->cancelled` is the case in which this
        // impl is owned by a Request object that exited from the scope
        if (*impl->cancelled) {
            delete impl;
            return;
        }

        impl->callback(Response(code, type, count,
            ttl, impl->ticks, addresses));

        delete impl;
    }

    in_addr *ipv4_pton(std::string address, in_addr *netaddr) {
        if (libevent->inet_pton(AF_INET, address.c_str(), netaddr) != 1) {
            throw std::runtime_error("Invalid IPv4 address");
        }
        return (netaddr);
    }

    in6_addr *ipv6_pton(std::string address, in6_addr *netaddr) {
        if (libevent->inet_pton(AF_INET6, address.c_str(), netaddr) != 1) {
            throw std::runtime_error("Invalid IPv6 address");
        }
        return (netaddr);
    }

    // Declared explicitly as private so one cannot delete this object
    ~RequestImpl() {
        // Nothing to see here, move along :)
    }

    // Private to enforce usage through issue()
    RequestImpl(std::string query, std::string address,
                std::function<void(Response&&)>&& f, evdns_base *base,
                IghtLibevent *lev, SharedPointer<bool> cancd)
            : callback(f), libevent(lev), cancelled(cancd) {

        assert(base != NULL && lev != NULL);

        //
        // We explain above why we don't store the return value
        // of the evdns_base_resolve_xxx() functions below
        //
        if (query == "A") {
            if (libevent->evdns_base_resolve_ipv4(base, address.c_str(),
                DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "AAAA") {
            if (libevent->evdns_base_resolve_ipv6(base, address.c_str(),
                DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_A") {
            in_addr na;
            if (libevent->evdns_base_resolve_reverse(base, ipv4_pton(address,
                &na), DNS_QUERY_NO_SEARCH, handle_resolve, this) == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_AAAA") {
            in6_addr na;
            if (libevent->evdns_base_resolve_reverse_ipv6(base, ipv6_pton(
                address, &na), DNS_QUERY_NO_SEARCH, handle_resolve, this)
                == NULL) {
                throw std::runtime_error("Resolver error");
            }
        } else {
            throw std::runtime_error("Unsupported query");
        }

        ticks = ight_time_now();
    }

public:
    static void issue(std::string query, std::string address,
                      std::function<void(Response&&)>&& func, evdns_base *base,
                      IghtLibevent *lev, SharedPointer<bool> cancd) {
        new RequestImpl(query, address, std::move(func), base, lev, cancd);
    }
};

}}}  // namespace

//
// Async DNS request.
//

Request::Request(std::string query, std::string address,
                       std::function<void(Response&&)>&& func,
                       evdns_base *dnsb,
                       IghtLibevent *libevent)
{
    if (dnsb == NULL) {
        dnsb = ight_get_global_evdns_base();
    }
    if (libevent == NULL) {
        libevent = IghtGlobalLibevent::get();
    }
    cancelled = std::make_shared<bool>();
    *cancelled = false;
    RequestImpl::issue(query, address, std::move(func),
                       dnsb, libevent, cancelled);
}

void
Request::cancel(void)
{
    if (cancelled) {  // May not be set when we used the default constructor
        *cancelled = true;
    }
}

//
// DNS Resolver object.
//

void
Resolver::cleanup(void)
{
    if (base != NULL) {
        //
        // Note: `1` means that pending requests are notified that
        // this evdns_base is being closed, i.e., their callback is
        // called with error DNS_ERROR_SHUTDOWN.
        //
        // We need to call evdns_base_free() like this because
        // this guarantees that request's callback is always invoked
        // so RequestImpl:s are always freed (see request()).
        //
        libevent->evdns_base_free(base, 1);
        base = NULL;  // Idempotent
    }
}

evdns_base *
Resolver::get_evdns_base(void)
{
    if (base != NULL) {  // Idempotent
        return base;
    }

    //
    // Note: in case of error, the object state is reset
    // like nothing has happened.
    //

    auto evb = poller->get_event_base();
    if (settings.find("nameserver") != settings.end()) {
        if ((base = libevent->evdns_base_new(evb, 0)) == NULL) {
            throw std::bad_alloc();
        }
        if (libevent->evdns_base_nameserver_ip_add(base,
                settings["nameserver"].c_str()) != 0) {
            cleanup();
            throw std::runtime_error("Cannot set server address");
        }
    } else if ((base = libevent->evdns_base_new(evb, 1)) == NULL) {
        throw std::bad_alloc();
    }

    if (settings.find("attempts") != settings.end() &&
            libevent->evdns_base_set_option(base,
            "attempts", settings["attempts"].c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'attempts' option");
    }
    if (settings.find("timeout") != settings.end() &&
            libevent->evdns_base_set_option(
            base, "timeout", settings["timeout"].c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'timeout' option");
    }

    // By default we don't randomize the query's case
    std::string randomiz{"0"};
    if (settings.find("randomize_case") != settings.end()) {
        randomiz = settings["randomize_case"];
    }
    if (libevent->evdns_base_set_option(base, "randomize-case",
            randomiz.c_str()) != 0) {
        cleanup();
        throw std::runtime_error("Cannot set 'randomize-case' option");
    }

    return base;
}

void
Resolver::request(std::string query, std::string address,
        std::function<void(Response&&)>&& func)
{
    //
    // Note: RequestImpl implements the autodelete behavior, meaning that
    // it shall delete itself once its callback is called. The callback
    // should always be called, either because of a successful response,
    // or because of an error, or because the resolver is destroyed (this
    // is guaranteed by the destructor's impl).
    //
    auto cancelled = std::make_shared<bool>();
    *cancelled = false;
    RequestImpl::issue(query, address, std::move(func),
                       get_evdns_base(), libevent, cancelled);
}
