// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_CONNECT_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_CONNECT_HPP

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/dns/resolve_hostname.hpp"
#include "src/libmeasurement_kit/ext/tls_internal.h"

#include <measurement_kit/net.hpp>

#include <event2/bufferevent.h>

#include <sstream>

struct bufferevent;
struct ssl_st;

extern "C" {
void mk_bufferevent_on_event(bufferevent *, short, void *);
}

namespace mk {
namespace net {

class ConnectResult {
  public:
    dns::ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    double connect_time = 0.0;
    bufferevent *connected_bev = nullptr;
};

typedef std::function<void(std::vector<Error>, bufferevent *)> ConnectFirstOfCb;

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

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl, std::string hostname,
                 Callback<Error, bufferevent *> cb,
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
