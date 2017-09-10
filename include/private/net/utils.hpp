// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_UTILS_HPP
#define PRIVATE_NET_UTILS_HPP

#include <measurement_kit/net.hpp>

struct sockaddr_storage;

namespace mk {
namespace net {

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        const char *family,
        const char *address,
        const char *port,
        Var<Logger> logger
);

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        int family,
        const char *address,
        const char *port,
        Var<Logger> logger
);

int storage_init(
        sockaddr_storage *storage,
        socklen_t *length,
        int family,
        const char *address,
        int port,
        Var<Logger> logger
);

os_socket_t socket_create(
        int domain,
        int type,
        int protocol,
        Var<Logger> logger
);

std::string unreverse_ipv6(std::string s);

std::string unreverse_ipv4(std::string s);

Error disable_nagle(os_socket_t);

Error map_errno(int);

} // namespace net
} // namespace mk
#endif
