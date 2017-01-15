// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_UTILS_HPP
#define MEASUREMENT_KIT_NET_UTILS_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace net {

struct Endpoint {
    std::string hostname;
    uint16_t port;
};

bool is_ipv4_addr(std::string s);
bool is_ipv6_addr(std::string s);
bool is_ip_addr(std::string s);

ErrorOr<Endpoint> parse_endpoint(std::string s, uint16_t def_port);
std::string serialize_endpoint(Endpoint);

} // namespace net
} // namespace mk
#endif
