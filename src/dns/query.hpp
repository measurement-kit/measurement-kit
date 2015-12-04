// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_QUERY_HPP
#define MEASUREMENT_KIT_DNS_QUERY_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/var.hpp>

#include <measurement_kit/dns/defines.hpp>

#include <functional>
#include <iosfwd>

struct evdns_base; // Internally we use evdns

namespace mk {

struct Libs;

namespace dns {

class Response;

/// Async DNS request.
class Query {

  protected:
    Var<bool> cancelled;

  public:
    /// Default constructor.
    Query() {}

    /// Start an async DNS request.
    Query(QueryClass dns_class, QueryType dns_type, std::string name,
          std::function<void(Error, Response)> func,
          Logger *lp = Logger::global(), evdns_base *dnsb = nullptr,
          Libs *libs = nullptr);

    /*
     * Following the rule of the three, we have a fancy destructor and we
     * need to also define all the following operations.
     *
     * The copy operations are deleted and only moving is allowed. This is
     * because the destructor sets to `false` the shared flag when exits the
     * scope. Hence we want only one Query bound to a QueryImpl at a time,
     * otherwise, in a scenario with multiple copies, when the first copy of
     * Query leaves the scope, the flag is the to `false`, and the callback
     * is not invoked even though one or more Queries are still alive.
     *
     * A better implementation would allow both copying and moving, and
     * would callback only if the shared pointer count is greater than one,
     * i.e. when at least a Query object is alive.
     *
     * Apparently asking for default move constructor is needed to enforce
     * the move constructor of the Var<T> as opposed to a constructor
     * that only trivially copies. This was noticed removing the four lines
     * and acknowledging that DNS code was not working anymore.
     */
    Query(Query &) = delete;
    Query &operator=(Query &) = delete;
    Query(Query &&) = default;
    Query &operator=(Query &&) = default;

    /// Cancel the pending Query.
    void cancel();

    /// Destructor.
    ~Query() { cancel(); }
};

} // namespace dns
} // namespace mk
#endif
