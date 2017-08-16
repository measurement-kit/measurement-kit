// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_CONNECT_HPP
#define MEASUREMENT_KIT_NET_CONNECT_HPP

#include <measurement_kit/dns.hpp>
#include <measurement_kit/net/transport.hpp>

struct bufferevent;

namespace mk {
namespace net {

void connect(std::string address, int port,
             Callback<Error, Var<Transport>> callback,
             Settings settings = {},
             Var<Reactor> reactor = Reactor::global(),
             Var<Logger> logger = Logger::global());

using ConnectManyCb = Callback<Error, std::vector<Var<Transport>>>;

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
