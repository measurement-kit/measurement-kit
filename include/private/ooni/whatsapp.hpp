// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_OONI_WHATSAPP_HPP
#define PRIVATE_OONI_WHATSAPP_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

uint32_t parseIPV4string(std::string ipAddress);
std::vector<uint8_t> ip_to_bytes(std::string ip);
ErrorOr<bool> same_pre(std::vector<uint8_t> ip1, std::vector<uint8_t> ip2, int pre_bits);
ErrorOr<bool> ip_in_net(std::string ip1, std::string ip_w_mask);
bool ip_in_nets(std::string ip, std::vector<std::string> nets);

} // namespace ooni
} // namespace mk

#endif
