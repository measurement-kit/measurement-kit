// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_CONNECT_HPP
#define MEASUREMENT_KIT_NET_CONNECT_HPP

#include <measurement_kit/net/transport.hpp>

struct bufferevent;

namespace mk {
namespace net {

void connect(std::string address, int port,
             Callback<Error, SharedPtr<Transport>> callback,
             Settings settings = {},
             SharedPtr<Reactor> reactor = Reactor::global(),
             SharedPtr<Logger> logger = Logger::global());

using ConnectManyCb = Callback<Error, std::vector<SharedPtr<Transport>>>;

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
