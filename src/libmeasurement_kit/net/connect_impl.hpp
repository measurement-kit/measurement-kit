// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_CONNECT_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_CONNECT_IMPL_HPP

#include <measurement_kit/net.hpp>

#include <event2/bufferevent.h>

#include <cerrno>
#include <sstream>

#include "../common/utils.hpp"
#include "../net/connect.hpp"
#include "../net/utils.hpp"

struct bufferevent;

extern "C" {
void mk_bufferevent_on_event(bufferevent *, short, void *);
}

namespace mk {
namespace net {

template <MK_MOCK(evutil_parse_sockaddr_port), MK_MOCK(bufferevent_socket_new),
          MK_MOCK(bufferevent_set_timeouts),
          MK_MOCK(bufferevent_socket_connect)>
void connect_base(std::string address, int port,
                  Callback<Error, bufferevent *, double> cb,
                  double timeout = 10.0,
                  Var<Reactor> reactor = Reactor::global(),
                  Var<Logger> logger = Logger::global()) {
    logger->debug("connect_base %s:%d", address.c_str(), port);
    Var<ConnectBaseResult> cbr{new ConnectBaseResult};
    cbr->address = address;
    cbr->port = port;

    std::string endpoint = [&]() {
        Endpoint endpoint;
        endpoint.hostname = address;
        endpoint.port = port;
        return serialize_endpoint(endpoint);
    }();

    sockaddr_storage storage;
    sockaddr *saddr = (sockaddr *)&storage;
    int salen = sizeof storage;

    // XXX: as we have seen in #915, the following function does not deal with
    // IPv6 scoped link local addresses. We can fix this by copying from the
    // way in which such problem is solved in libevent/dns_utils.hpp.
    if (evutil_parse_sockaddr_port(endpoint.c_str(), saddr, &salen) != 0) {
        logger->warn("cannot parse endpoint: '%s'", endpoint.c_str());
        cb(GenericError(), nullptr, 0.0);
        return;
    }

    /*
     *  Rationale for deferring callbacks:
     *
     *  When using IOCP on Windows, the kernel calls callbacks when selected
     *  events occur (i.e., there is no loop that guarantees callbacks run in
     *  the same thread); set DEFER_CALLBACKS to tell libevent to serialize
     *  bufferevent's callbacks into the event loop to avoid creating MT issues
     *  in code that otherwise (on Unices) is single threaded.
     *
     *  Yes, the current implementation forces serializing the callbacks also
     *  on Unix where this wouldn't be needed thus adding some overhead. For
     *  uniformity, I am for serializing for all platforms and then, if we see
     *  that there's too much overhead, to only enable that on Windows.
     */
    static const int flags = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;

    bufferevent *bev;
    if ((bev = bufferevent_socket_new(reactor->get_event_base(), -1,
                                      flags)) == nullptr) {
        throw GenericError(); // This should not happen
    }

    timeval tv, *tvp = timeval_init(&tv, timeout);
    if (bufferevent_set_timeouts(bev, tvp, tvp) != 0) {
        bufferevent_free(bev);
        throw GenericError(); // This should not happen
    }

    double begin = mk::time_now();

    if (bufferevent_socket_connect(bev, saddr, salen) != 0) {
        bufferevent_free(bev);
        Error sys_error = mk::net::map_errno(errno);
        logger->warn("reason why connect() has failed: %s",
                     sys_error.as_ooni_error().c_str());
        sys_error.context = cbr;
        cb(sys_error, nullptr, 0.0);
        return;
    }

    logger->debug("connect() in progress...");

    // WARNING: set callbacks after connect() otherwise we free `bev` twice
    // NOTE: In case of `new` failure we let the stack unwind
    bufferevent_setcb(
        bev, nullptr, nullptr, mk_bufferevent_on_event,
        new Callback<Error, bufferevent *>([=](Error err, bufferevent *bev) {
            if (err) {
                bufferevent_free(bev);
                logger->warn("reason why connect() has failed: %s",
                             err.as_ooni_error().c_str());
                err = ConnectFailedRemotelyError(err);
                err.context = cbr;
                cb(err, nullptr, 0.0);
                return;
            }
            double elapsed = mk::time_now() - begin;
            logger->debug("connect time: %f", elapsed);
            err.context = cbr;
            cb(err, bev, elapsed);
        }));
}

template <MK_MOCK_NAMESPACE(net, connect)>
void connect_many_impl(Var<ConnectManyCtx> ctx) {
    // Implementation note: this function connects sequentially, which
    // is slower but also much simpler to implement and verify
    if (ctx->left <= 0) {
        Error err = NoError();
        err.context = ctx->result;
        ctx->callback(err, ctx->connections);
        return;
    }
    net_connect(ctx->address, ctx->port,
                [=](Error err, Var<Transport> txp) {
                    if (err) {
                        ctx->callback(err, ctx->connections);
                        return;
                    }
                    Var<ConnectResult> cr = err.context.as<ConnectResult>();
                    if (!!cr) {
                        ctx->result->results.push_back(cr);
                    }
                    ctx->connections.push_back(std::move(txp));
                    --ctx->left;
                    connect_many_impl<net_connect>(ctx);
                },
                ctx->settings, ctx->reactor, ctx->logger);
}

static inline Var<ConnectManyCtx>
connect_many_make(std::string address, int port, int count,
                  ConnectManyCb callback, Settings settings,
                  Var<Reactor> reactor, Var<Logger> logger) {
    Var<ConnectManyCtx> ctx(new ConnectManyCtx);
    ctx->left = count;
    ctx->callback = callback;
    ctx->address = address;
    ctx->port = port;
    ctx->settings = settings;
    ctx->reactor = reactor;
    ctx->logger = logger;
    ctx->result.reset(new ConnectManyResult);
    ctx->result->hostname = address;
    ctx->result->port = port;
    return ctx;
}

} // namespace mk
} // namespace net
#endif
