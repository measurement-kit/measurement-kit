// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_CONNECT_HPP
#define MEASUREMENT_KIT_NET_CONNECT_HPP

#include <measurement_kit/dns.hpp>
#include <measurement_kit/net/transport.hpp>

struct bufferevent;

namespace mk {
namespace net {

class ConnectResult : public ErrorContext {
  public:
    dns::ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    double connect_time = 0.0;
    bufferevent *connected_bev = nullptr;
    ~ConnectResult() override;
};

// Convert error returned by connect() in connect_time
ErrorOr<double> get_connect_time(Error error);

void connect(std::string address, int port,
             Callback<Error, Var<Transport>> callback,
             Settings settings = {},
             Var<Reactor> reactor = Reactor::global(),
             Var<Logger> logger = Logger::global());

class ConnectManyResult : public ErrorContext {
  public:
    std::vector<Var<ConnectResult>> results;
    ~ConnectManyResult() override;
};

// Convert error returned by connect_many() into connect_times vector
ErrorOr<std::vector<double>> get_connect_times(Error error);

using ConnectManyCb = Callback<Error, std::vector<Var<Transport>>>;

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
