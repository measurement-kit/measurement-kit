// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECT_MANY_HPP
#define SRC_NET_CONNECT_MANY_HPP

#include <functional>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <string>
#include <vector>

namespace mk {
namespace net {

class ConnectManyCtx {
  public:
    int left = 0; // Signed to detect programmer errors
    ConnectManyCb callback;
    std::vector<Var<Transport>> connections;
    std::string address;
    int port = 0;
    Settings settings;
    Logger *logger = Logger::global();
    Poller *poller = Poller::global();
};

template <void (*do_connect)(std::string, int,
              Callback<Var<Transport>>, Settings, Logger *,
              Poller *) = net::connect>
static void connect_many_(Var<ConnectManyCtx> ctx) {
    // Implementation note: this function connects sequentially, which
    // is slower but also much simpler to implement and verify
    if (ctx->left <= 0) {
        ctx->callback(NoError(), ctx->connections);
        return;
    }
    do_connect(ctx->address, ctx->port,
        [=](Error err, Var<Transport> txp) {
            if (err) {
                ctx->callback(err, ctx->connections);
                return;
            }
            ctx->connections.push_back(txp);
            --ctx->left;
            connect_many_<do_connect>(ctx);
        },
        ctx->settings, ctx->logger, ctx->poller);
}

static Var<ConnectManyCtx> connect_many_make(std::string address, int port,
        int count, ConnectManyCb callback, Settings settings, Logger *logger,
        Poller *poller) {
    Var<ConnectManyCtx> ctx(new ConnectManyCtx);
    ctx->left = count;
    ctx->callback = callback;
    ctx->address = address;
    ctx->port = port;
    ctx->settings = settings;
    ctx->logger = logger;
    ctx->poller = poller;
    return ctx;
}

} // namespace net
} // namespace mk
#endif
