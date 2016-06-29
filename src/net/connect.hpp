// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECT_HPP
#define SRC_NET_CONNECT_HPP

#include "src/ext/tls_internal.h"

#include "event2/util.h"
#include "src/common/utils.hpp"
#include <arpa/inet.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <functional>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <vector>

struct bufferevent;
struct ssl_st;

extern "C" {
void mk_bufferevent_on_event(bufferevent *, short, void *);
}

namespace mk {
namespace net {

typedef std::function<void(std::vector<Error>, bufferevent *)> ConnectFirstOfCb;

void connect_first_of(std::vector<std::string> addresses, int port,
                      ConnectFirstOfCb cb, Settings settings = {},
                      Var<Reactor> reactor = Reactor::global(),
                      Var<Logger> logger = Logger::global(), size_t index = 0,
                      Var<std::vector<Error>> errors = nullptr);

typedef std::function<void(ResolveHostnameResult)> ResolveHostnameCb;

void resolve_hostname(std::string hostname, ResolveHostnameCb cb,
                      Settings settings = {},
                      Var<Reactor> reactor = Reactor::global(),
                      Var<Logger> logger = Logger::global());

void connect_logic(std::string hostname, int port,
                   Callback<Error, Var<ConnectResult>> cb,
                   Settings settings = {},
                   Var<Reactor> reactor = Reactor::global(),
                   Var<Logger> logger = Logger::global());

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl, std::string hostname,
                 Callback<Error, bufferevent *> cb,
                 Var<Reactor> = Reactor::global(),
                 Var<Logger> = Logger::global());

class ConnectManyCtx {
  public:
    int left = 0; // Signed to detect programmer errors
    ConnectManyCb callback;
    std::vector<Var<Transport>> connections;
    std::string address;
    int port = 0;
    Settings settings;
    Var<Logger> logger = Logger::global();
    Var<Reactor> reactor = Reactor::global();
};

} // namespace mk
} // namespace net
#endif
