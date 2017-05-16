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

// Proxy required because `make_sockaddr` is overloaded
static Error make_sockaddr_proxy(std::string s, std::string p,
                                 sockaddr_storage *ss, socklen_t *len) {
    return make_sockaddr(s, p, ss, len);
}

template <MK_MOCK(make_sockaddr_proxy), MK_MOCK(bufferevent_socket_new),
          MK_MOCK(bufferevent_set_timeouts),
          MK_MOCK(bufferevent_socket_connect)>
void connect_base(std::string address, int port,
                  Callback<Error, bufferevent *, double> cb,
                  double timeout = 10.0,
                  Var<Reactor> reactor = Reactor::global(),
                  Var<Logger> logger = Logger::global()) {

    std::string endpoint = [&]() {
        Endpoint endpoint;
        endpoint.hostname = address;
        endpoint.port = (uint16_t)port; /* XXX We should change the prototype */
        return serialize_endpoint(endpoint);
    }();
    logger->debug("connect_base %s", endpoint.c_str());

    std::string port_string = std::to_string(port);
    sockaddr_storage storage = {};
    socklen_t salen = 0;
    Error err = make_sockaddr_proxy(address, port_string, &storage, &salen);
    if (err != NoError()) {
        logger->warn("cannot parse endpoint: '%s'", endpoint.c_str());
        cb(err, nullptr, 0.0);
        return;
    }
    sockaddr *saddr = (sockaddr *)&storage;

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
        logger->warn("connect() for %s failed immediately", endpoint.c_str());
        bufferevent_free(bev);
        Error sys_error = mk::net::map_errno(errno);
        if (sys_error == NoError()) {
            sys_error = GenericError(); /* We must report an error */
        }
        logger->warn("reason why connect() has failed: %s",
                     sys_error.as_ooni_error().c_str());
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
                logger->warn("connect() for %s failed in its callback",
                             endpoint.c_str());
                bufferevent_free(bev);
                logger->warn("reason why connect() has failed: %s",
                             err.as_ooni_error().c_str());
                cb(err, nullptr, 0.0);
                return;
            }
            double elapsed = mk::time_now() - begin;
            logger->debug("connect time: %f", elapsed);
            cb(err, bev, elapsed);
        }));
}

template <MK_MOCK_AS(net::connect, net_connect)>
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
    return ctx;
}

} // namespace mk
} // namespace net
#endif
