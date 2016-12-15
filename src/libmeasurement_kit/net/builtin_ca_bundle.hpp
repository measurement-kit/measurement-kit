// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_BUILTIN_CA_BUNDLE_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_BUILTIN_CA_BUNDLE_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace mk {
namespace net {

std::vector<uint8_t> builtin_ca_bundle();

} // namespace net
} // namespace mk
#endif
