// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_CONNECT_IMPL_HPP
#define PRIVATE_NET_CONNECT_IMPL_HPP

#include "private/common/mock.hpp"
#include "private/common/utils.hpp"

#include <measurement_kit/net.hpp>

#include <event2/bufferevent.h>

#include <cerrno>
#include <sstream>

#include "../net/connect.hpp"
#include "../net/utils.hpp"

struct bufferevent;

extern "C" {
void mk_bufferevent_on_event(bufferevent *, short, void *);
}

namespace mk {
namespace net {

template <MK_MOCK_AS(net::connect, net_connect)>
void connect_many_impl(SharedPtr<ConnectManyCtx> ctx) {
    // Implementation note: this function connects sequentially, which
    // is slower but also much simpler to implement and verify
    if (ctx->left <= 0) {
        Error err = NoError();
        ctx->callback(err, ctx->connections);
        return;
    }
    net_connect(ctx->address, ctx->port,
                [=](Error err, SharedPtr<Transport> txp) {
                    ctx->connections.push_back(std::move(txp));
                    if (err) {
                        ctx->callback(err, ctx->connections);
                        return;
                    }
                    --ctx->left;
                    connect_many_impl<net_connect>(ctx);
                },
                ctx->settings, ctx->reactor, ctx->logger);
}

static inline SharedPtr<ConnectManyCtx>
connect_many_make(std::string address, int port, int count,
                  ConnectManyCb callback, Settings settings,
                  SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<ConnectManyCtx> ctx(new ConnectManyCtx);
    ctx->left = count;
    ctx->callback = callback;
    ctx->address = address;
    ctx->port = port;
    ctx->settings = settings;
    ctx->reactor = reactor;
    ctx->logger = logger;
    return ctx;
}

} // namespace mk
} // namespace net
#endif
