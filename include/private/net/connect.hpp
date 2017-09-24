// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_CONNECT_HPP
#define PRIVATE_NET_CONNECT_HPP

#include "private/net/emitter.hpp"
#include "private/common/utils.hpp"
#include "../ext/tls_internal.h"

#include <measurement_kit/net.hpp>

#include <event2/bufferevent.h>

#include <sstream>

struct bufferevent;
struct ssl_st;

namespace mk {
namespace net {

class ConnectResult {
  public:
    dns::ResolveHostnameResult resolve_result;
    SharedPtr<Transport> connected_txp;
};

typedef std::function<void(Error, std::vector<Error>)> ConnectFirstOfCb;

void connect_first_of(SharedPtr<ConnectResult> result, int port,
                      ConnectFirstOfCb cb, Settings settings = {},
                      SharedPtr<Reactor> reactor = Reactor::global(),
                      SharedPtr<Logger> logger = Logger::global(), size_t index = 0,
                      SharedPtr<std::vector<Error>> errors = nullptr);

void connect_logic(std::string hostname, int port,
                   Callback<Error, SharedPtr<ConnectResult>> cb,
                   Settings settings = {},
                   SharedPtr<Reactor> reactor = Reactor::global(),
                   SharedPtr<Logger> logger = Logger::global());

void connect_ssl(SharedPtr<ConnectResult> r, ssl_st *ssl, std::string hostname,
                 Callback<Error> cb,
                 SharedPtr<Reactor> = Reactor::global(),
                 SharedPtr<Logger> = Logger::global());

class ConnectManyCtx {
  public:
    int left = 0; // Signed to detect programmer errors
    ConnectManyCb callback;
    std::vector<SharedPtr<Transport>> connections;
    std::string address;
    int port = 0;
    Settings settings;
    SharedPtr<Reactor> reactor = Reactor::global();
    SharedPtr<Logger> logger = Logger::global();
};

} // namespace mk
} // namespace net
#endif
