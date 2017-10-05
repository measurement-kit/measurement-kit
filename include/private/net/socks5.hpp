// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_SOCKS5_HPP
#define PRIVATE_NET_SOCKS5_HPP

#include "../net/emitter.hpp"

#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

Buffer socks5_format_auth_request(SharedPtr<Logger> = Logger::global());

ErrorOr<bool> socks5_parse_auth_response(
        Buffer &, SharedPtr<Logger> = Logger::global());

ErrorOr<Buffer> socks5_format_connect_request(
        Settings, SharedPtr<Logger> = Logger::global());

ErrorOr<bool> socks5_parse_connect_response(
        Buffer &, SharedPtr<Logger> = Logger::global());

void socks5_connect(std::string address, int port, Settings settings,
        std::function<void(Error, SharedPtr<Transport>)> callback,
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
