// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_UTILS_HPP
#define MEASUREMENT_KIT_NET_UTILS_HPP

#include <measurement_kit/common.hpp>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

struct sockaddr_storage;

namespace mk {
namespace net {

class Endpoint {
  public:
    std::string hostname;
    uint16_t port = 0;
};

bool is_ipv4_addr(std::string s);
bool is_ipv6_addr(std::string s);
bool is_ip_addr(std::string s);

ErrorOr<Endpoint> parse_endpoint(std::string s, uint16_t def_port);
std::string serialize_endpoint(Endpoint);

ErrorOr<Endpoint> endpoint_from_sockaddr_storage(
        sockaddr_storage *storage
) noexcept;

Error make_sockaddr(
        std::string address,
        uint16_t port,
        sockaddr_storage *storage,
        socklen_t *len
) noexcept;

} // namespace net
} // namespace mk
#endif
