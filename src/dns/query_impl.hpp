// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_DNS_QUERY_IMPL_HPP
#define SRC_DNS_QUERY_IMPL_HPP

#include <measurement_kit/dns/response.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/utils.hpp>

#include <event2/dns.h>

#include <cassert>
#include <functional>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <netinet/in.h>
#include <sys/socket.h>

struct evdns_base;

namespace measurement_kit {
namespace dns {

using namespace measurement_kit::common;

/// Implementation of Query.
class QueryImpl {

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

    std::function<void(Response)> callback;
    double ticks = 0.0; // just to initialize to something
    Libs *libs;         // should not be nullptr (this is asserted below)
    SharedPointer<bool> cancelled;
    Logger *logger = Logger::global();

    static void handle_resolve(int code, char type, int count, int ttl,
                               void *addresses, void *opaque) {

        auto impl = static_cast<QueryImpl *>(opaque);

        // Tell the libevent layer we received a DNS response
        if (impl->libs->evdns_reply_hook) {
            impl->libs->evdns_reply_hook(code, type, count, ttl, addresses,
                                         opaque);
        }

        // Note: the case of `impl->cancelled` is the case in which this
        // impl is owned by a Query object that exited from the scope
        if (*impl->cancelled) {
            delete impl;
            return;
        }

        impl->callback(Response(code, type, count, ttl, impl->ticks, addresses,
                                impl->logger));

        delete impl;
    }

    in_addr *ipv4_pton(std::string address, in_addr *netaddr) {
        if (libs->inet_pton(AF_INET, address.c_str(), netaddr) != 1) {
            throw std::runtime_error("Invalid IPv4 address");
        }
        return (netaddr);
    }

    in6_addr *ipv6_pton(std::string address, in6_addr *netaddr) {
        if (libs->inet_pton(AF_INET6, address.c_str(), netaddr) != 1) {
            throw std::runtime_error("Invalid IPv6 address");
        }
        return (netaddr);
    }

    // Declared explicitly as private so one cannot delete this object
    ~QueryImpl() {}

    // Private to enforce usage through issue()
    QueryImpl(std::string query, std::string address,
              std::function<void(Response)> f, Logger *lp,
              evdns_base *base, Libs *lev, SharedPointer<bool> cancd)
        : callback(f), libs(lev), cancelled(cancd), logger(lp) {

        assert(base != nullptr && lev != nullptr);

        //
        // We explain above why we don't store the return value
        // of the evdns_base_resolve_xxx() functions below
        //
        if (query == "A") {
            if (libs->evdns_base_resolve_ipv4(
                    base, address.c_str(), DNS_QUERY_NO_SEARCH, handle_resolve,
                    this) == nullptr) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "AAAA") {
            if (libs->evdns_base_resolve_ipv6(
                    base, address.c_str(), DNS_QUERY_NO_SEARCH, handle_resolve,
                    this) == nullptr) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_A") {
            in_addr na;
            if (libs->evdns_base_resolve_reverse(
                    base, ipv4_pton(address, &na), DNS_QUERY_NO_SEARCH,
                    handle_resolve, this) == nullptr) {
                throw std::runtime_error("Resolver error");
            }
        } else if (query == "REVERSE_AAAA") {
            in6_addr na;
            if (libs->evdns_base_resolve_reverse_ipv6(
                    base, ipv6_pton(address, &na), DNS_QUERY_NO_SEARCH,
                    handle_resolve, this) == nullptr) {
                throw std::runtime_error("Resolver error");
            }
        } else {
            throw std::runtime_error("Unsupported query");
        }

        ticks = measurement_kit::time_now();
    }

  public:
    static void issue(std::string query, std::string address,
                      std::function<void(Response)> func, Logger *logger,
                      evdns_base *base, Libs *lev, SharedPointer<bool> cancd) {
        new QueryImpl(query, address, func, logger, base, lev, cancd);
    }
};

} // namespace dns
} // namespace measurement_kit
#endif
