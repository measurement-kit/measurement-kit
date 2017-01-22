// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_ARES_MAP_FAILURE_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_ARES_MAP_FAILURE_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

Error ares_map_failure(int status);

} // namespace dns
} // namespace mk
#endif
