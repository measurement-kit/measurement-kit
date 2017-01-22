// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

Error eai_to_error(int eai);

/*
 * Design notes:
 *
 * Since the `addrinfo` is not very often shared, an alternative approach
 * here could be that of using a `unique_ptr`. I tried to use it, but I've
 * noticed that ErrorOr constructor does not cope well with the std::move
 * needed to move around a unique_ptr, so stopped the experiment early.
 *
 * At the same time, MeasurementKit already uses Var<> in many places
 * with the assumption that you will want to pass variables to lambdas
 * using lambda closure; for this reason, it probably make sense to
 * keep using Var<> here even though we're losing something in purity.
 *
 * - - -
 *
 * Implementation note: here I'm using `const char *` as opposed to
 * `std::string` to allow for the `nullptr` argument used to indicate
 * to getaddrinfo() that the address (or port) is not specified.
 *
 * Potential for refactoring:
 *
 * 1. other code that uses getaddrinfo() may want to use this API
 *
 * 2. other code that maps strings IP addresses to `sockaddr` may want
 *    to use this code as well
 *
 * 3. we _may_ want to use `evutil_getaddrinfo()` rather than just
 *    `getaddrinfo()` in the underlying code (I am not sure as it
 *    seems that `evutil_getaddrinfo()` targets backward compat with
 *    old systems and perhaps I'd like to reduce the amount of code
 *    we have that depends on libevent. At the same time, the API
 *    does not expose any libevent specifics so it can be done.
 */

ErrorOr<Var<addrinfo>> getaddrinfo(const char *hostname, const char *port,
                                   addrinfo *hints, Var<Logger>);

ErrorOr<Var<addrinfo>> getaddrinfo_numeric_datagram(const char *hostname,
                                                    const char *port,
                                                    Var<Logger> logger);

ErrorOr<Var<addrinfo>> getaddrinfo_numeric_stream(const char *hostname,
                                                  const char *port,
                                                  Var<Logger> logger);

} // namespace dns
} // namespace mk
#endif
