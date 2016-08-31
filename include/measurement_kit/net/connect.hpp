// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_CONNECT_HPP
#define MEASUREMENT_KIT_NET_CONNECT_HPP

#include <measurement_kit/net/transport.hpp>
#include <vector>

namespace mk {
namespace net {

struct ResolveHostnameResult {
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;

    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;

    std::vector<std::string> addresses;
};

struct ConnectResult : public ErrorContext {
    ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    bufferevent *connected_bev = nullptr;
};

void connect(std::string address, int port,
             Callback<Error, Var<Transport>> callback,
             Settings settings = {},
             Var<Logger> logger = Logger::global(),
             Var<Reactor> reactor = Reactor::global());

using ConnectManyCb = Callback<Error, std::vector<Var<Transport>>>;

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        Var<Logger> logger = Logger::global(),
        Var<Reactor> reactor = Reactor::global());

} // namespace net
} // namespace mk
#endif
