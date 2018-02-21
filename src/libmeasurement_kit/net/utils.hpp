// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_UTILS_HPP

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include <measurement_kit/common.hpp>

struct sockaddr_storage;

namespace mk {
namespace net {

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        const char *family,
        const char *address,
        const char *port,
        SharedPtr<Logger> logger
);

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        int family,
        const char *address,
        const char *port,
        SharedPtr<Logger> logger
);

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        int family,
        const char *address,
        int port,
        SharedPtr<Logger> logger
);

socket_t socket_create(
        int domain,
        int type,
        int protocol,
        SharedPtr<Logger> logger
);

std::string unreverse_ipv6(std::string s);

std::string unreverse_ipv4(std::string s);

Error disable_nagle(socket_t);

Error map_errno(int);

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
