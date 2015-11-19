// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_DNS_QUERY_IMPL_HPP
#define SRC_DNS_QUERY_IMPL_HPP

#include <measurement_kit/dns/defines.hpp>
#include <measurement_kit/dns/error.hpp>
#include <measurement_kit/dns/response.hpp>

#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/var.hpp>
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

    std::function<void(Error, Response)> callback;
    double ticks = 0.0; // just to initialize to something
    Libs *libs;         // should not be nullptr (this is asserted below)
    Var<bool> cancelled;
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

        // The constructor of Response performs additional checks and may
        // override the original code on ill-formed responses.
        // For this reason below we check `get_evdns_code()` rather
        // than just checking the value of `code`.
        Response resp(code, type, count, ttl, impl->ticks,
                      addresses, impl->logger);
        if (resp.get_evdns_status() != DNS_ERR_NONE) {
            impl->callback(measurement_kit::dns::dns_error(
                resp.get_evdns_status()), resp);
        } else
            impl->callback(common::NoError(), resp);

        delete impl;
    }

    in_addr *ipv4_pton(std::string address, in_addr *netaddr) {
        if (libs->inet_pton(AF_INET, address.c_str(), netaddr) != 1) {
            throw InvalidIPv4AddressError();
        }
        return (netaddr);
    }

    in6_addr *ipv6_pton(std::string address, in6_addr *netaddr) {
        if (libs->inet_pton(AF_INET6, address.c_str(), netaddr) != 1) {
            throw InvalidIPv6AddressError();
        }
        return (netaddr);
    }

    // Declared explicitly as private so one cannot delete this object
    ~QueryImpl() {}

    // Private to enforce usage through issue()
    QueryImpl(QueryClass dns_class, QueryType dns_type, std::string name,
              std::function<void(Error, Response)> f, Logger *lp,
              evdns_base *base, Libs *lev, Var<bool> cancd)
        : callback(f), libs(lev), cancelled(cancd), logger(lp) {

        assert(base != nullptr && lev != nullptr);
        if (dns_class != QueryClassId::IN) throw UnsupportedClassError();

        // Allow PTR queries
        if (dns_type == QueryTypeId::PTR) {
            std::string s;
            if ((s = measurement_kit::unreverse_ipv4(name)) != "") {
                dns_type = QueryTypeId::REVERSE_A;
                name = s;
            } else if ((s = measurement_kit::unreverse_ipv6(name)) != "") {
                dns_type = QueryTypeId::REVERSE_AAAA;
                name = s;
            } else
                throw InvalidNameForPTRError();
        }

        //
        // We explain above why we don't store the return value
        // of the evdns_base_resolve_xxx() functions below
        //
        if (dns_type == QueryTypeId::A) {
            if (libs->evdns_base_resolve_ipv4(
                    base, name.c_str(), DNS_QUERY_NO_SEARCH, handle_resolve,
                    this) == nullptr) {
                throw ResolverError();
            }
        } else if (dns_type == QueryTypeId::AAAA) {
            if (libs->evdns_base_resolve_ipv6(
                    base, name.c_str(), DNS_QUERY_NO_SEARCH, handle_resolve,
                    this) == nullptr) {
                throw ResolverError();
            }
        } else if (dns_type == QueryTypeId::REVERSE_A) {
            in_addr na;
            if (libs->evdns_base_resolve_reverse(
                    base, ipv4_pton(name, &na), DNS_QUERY_NO_SEARCH,
                    handle_resolve, this) == nullptr) {
                throw ResolverError();
            }
        } else if (dns_type == QueryTypeId::REVERSE_AAAA) {
            in6_addr na;
            if (libs->evdns_base_resolve_reverse_ipv6(
                    base, ipv6_pton(name, &na), DNS_QUERY_NO_SEARCH,
                    handle_resolve, this) == nullptr) {
                throw ResolverError();
            }
        } else {
            throw UnsupportedTypeError();
        }

        ticks = measurement_kit::time_now();
    }

  public:
    static void issue(QueryClass dns_class, QueryType dns_type,
                      std::string name, std::function<void(Error, Response)> f,
                      Logger *logger, evdns_base *base, Libs *lev,
                      Var<bool> cancd) {
        try {
            new QueryImpl(dns_class, dns_type, name, f, logger,
                          base, lev, cancd);
        } catch (common::Error &error) {
            f(error, Response());
        } catch (...) {
            f(common::GenericError(), Response());
        }
    }
};

} // namespace dns
} // namespace measurement_kit
#endif
