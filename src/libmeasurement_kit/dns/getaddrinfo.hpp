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
 * Since the `addrinfo` is not very often shared, an alternative approach
 * here could be that of using a `unique_ptr`. I tried to use it, but I've
 * noticed that ErrorOr constructor does not cope well with the std::move
 * needed to move around a unique_ptr, so stopped the experiment early.
 *
 * At the same time, MeasurementKit already uses Var<> in many places
 * with the assumption that you will want to pass variables to lambdas
 * using lambda closure; for this reason, it probably make sense to
 * keep using Var<> here even though we're losing something in purity.
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
