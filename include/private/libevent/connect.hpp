// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_CONNECT_IMPL_HPP
#define PRIVATE_NET_CONNECT_IMPL_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef MK_CA_BUNDLE
#error "Missing MK_CA_BUNDLE definition"
#endif

#include "private/common/mock.hpp"
#include "private/libevent/connection.hpp"
#include "private/net/emitter.hpp"
#include "private/net/libssl.hpp"
#include <cerrno>
#include <event2/bufferevent.h>
#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/error_or.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/common/utils.hpp>
#include <measurement_kit/net/error.hpp>
#include <measurement_kit/net/transport.hpp>
#include <measurement_kit/net/transport.hpp>
#include <measurement_kit/portable/sys/socket.h>
#include <sstream>

extern "C" {
static inline void mk_libevent_connect(bufferevent *, short, void *);
}

namespace mk {
namespace libevent {

class Bev : public NonCopyable, public NonMovable {
  public:
    Bev(bufferevent *bev) : bev_{bev} {}

    ~Bev() {
        if (bev_ != nullptr) {
            bufferevent_free(bev_);
        }
    }

    bufferevent *get() const { return bev_; }

    bufferevent *release() {
        auto bev = bev_;
        bev_ = nullptr;
        return bev;
    }

    void set(bufferevent *bev) { bev_ = bev; }

  private:
    bufferevent *bev_ = nullptr;
};

template <MK_MOCK(bufferevent_socket_new), MK_MOCK(bufferevent_set_timeouts),
          MK_MOCK(bufferevent_socket_connect),
          MK_MOCK(bufferevent_openssl_filter_new),
          MK_MOCK_AS(libssl::verify_peer<>, libssl_verify_peer)>
void connect(sockaddr *saddr, socklen_t salen, Settings settings,
             Var<Reactor> reactor, Var<Logger> logger,
             Callback<Error, Var<net::Transport>> &&cb) {
    Var<Transport> txp{new Emitter{reactor, logger}};
    ErrorOr<double> timeout = settings.get_noexcept("net/timeout", 30.0);
    if (!timeout) {
        reactor->call_soon([=]() { cb(ValueError(), txp); });
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
    constexpr int flags = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;
    auto evbase = reactor->get_event_base();
    auto bev = Var<Bev>::make(bufferevent_socket_new(evbase, -1, flags));
    if (bev->get() == nullptr) {
        throw GenericError(); // This should not happen
    }
    timeval tv, *tvp = timeval_init(&tv, timeout);
    if (bufferevent_set_timeouts(bev->get(), tvp, tvp) != 0) {
        throw GenericError(); // This should not happen
    }
    double begin = mk::time_now();
    if (bufferevent_socket_connect(bev->get(), saddr, salen) != 0) {
        Error err = mk::net::map_errno(errno);
        if (err == NoError()) {
            err = GenericError(); /* We must report an error */
        }
        logger->warn("connect() failed: %s", err.as_ooni_error().c_str());
        reactor->call_soon([=]() { cb(err, txp); });
        return;
    }
    // NOTE: in case of `new` failure we let the stack unwind
    auto pcb = new Callback<short>{[=](short what) {
        double elapsed = mk::time_now() - begin;
        txp->set_connect_time_(elapsed);
        logger->debug("connect() callback; elapsed time: %f; bufferevent "
                      "event",
                      elapsed, map_bufferevent_event(what).c_str());
        if ((what & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) != 0) {
            Error err = ((what & BEV_EVENT_TIMEOUT) != 0)
                            ? TimeoutError()
                            : (errno) ? net::map_errno(errno) : GenericError();
            logger->warn("connect() error: %s", err.as_ooni_error().c_str());
            cb(err, txp);
            return;
        }
        ErrorOr<bool> ssl_enabled = settings.get_noexcept("net/ssl", false);
        if (!ssl_enabled) {
            logger->debug("connect() successful");
            cb(ValueError(), txp);
            return;
        }
        auto success = [elapsed]( //
                           Var<Bev> bev, Var<Reactor> reactor,
                           Var<Logger> logger,
                           Callback<Error, Var<net::Transport>> cb) {
            auto txp = Connection::make(bev->get(), reactor, logger);
            (void)bev->release();
            txp->set_connect_time_(elapsed);
            cb(NoError(), txp);
        };
        if (*ssl_enabled == false) {
            success(bev, reactor, logger, cb);
            return;
        }
        std::string bundle = settings.get("net/ca_bundle_path", MK_CA_BUNDLE);
        std::string hostname = settings.get("net/ssl_hostname", "");
        if (hostname == "") {
            cb(ValueError("missing 'net/ssl_hostname' setting"), txp);
            return;
        }
        ErrorOr<SSL *> ssl =
            libssl::Cache<>::thread_local_instance().get_client_ssl(
                bundle, hostname, logger);
        if (!ssl) {
            cb(ValueError(), txp);
            return;
        }
        ErrorOr<bool> allow23 = settings.get_noexcept("net/allow_ssl23", false);
        if (!allow23) {
            cb(ValueError(), txp);
            return;
        }
        if (*allow23 == true) {
            logger->info("Re-enabling SSLv2 and SSLv3");
            libssl::enable_v23(*ssl);
        }
        auto sslbev = bufferevent_openssl_filter_new(
            evbase, bev->get(), *ssl, BUFFEREVENT_SSL_CONNECTING, flags);
        if (sslbev == nullptr) {
            cb(GenericError(), txp);
            return;
        }
        (void)bev->release();
        bev->set(sslbev);
        auto x = new Callback<short>{[=](short what) {
            logger->debug("ssl handshake callback; bufferevent event: %s",
                          map_bufferevent_event(what).c_str());
            Error err;
            if ((what & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) != 0) {
                if ((what & BEV_EVENT_TIMEOUT) != 0) {
                    err = TimeoutError();
                } else {
                    err = net::map_errno(errno);
                    if (!err) {
                        long ssle = bufferevent_get_openssl_error(bev->get());
                        std::string s = ERR_error_string(ssle, nullptr);
                        err = mk::net::SslError(s);
                    }
                }
                logger->warn("ssl hanshake error: %s",
                             err.as_ooni_error().c_str());
                cb(err, txp);
                return;
            }
            err = libssl_verify_peer(hostname, *ssl, logger);
            if (err) {
                cb(err, txp);
                return;
            }
            ErrorOr<bool> allow_dirty_shutdown =
                settings.get_noexcept("net/ssl_allow_dirty_shutdown", false);
            if (!allow_dirty_shutdown) {
                cb(ValueError(), txp);
                return;
            }
            if (*allow_dirty_shutdown == true) {
#ifdef HAVE_BUFFEREVENT_OPENSSL_SET_ALLOW_DIRTY_SHUTDOWN
                bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
                logger->info("Allowing dirty SSL shutdown");
#else
                logger->warn("Cannot tell libevent to allow SSL dirty "
                             "shutdowns as requested by the programmer: as a "
                             "result some SSL connections may interrupt "
                             "abruptly with an error. This happens because you "
                             "are not using version 2.1.x of libevent.");
#endif
            }
            logger->debug("ssl: handshake... complete");
            success(bev, reactor, logger, cb);
        }};
        bufferevent_setcb(bev->get(), nullptr, nullptr, mk_libevent_connect, x);
        logger->debug("ssl handshake in progress...");
    }};
    // NOTE: we set callback after `connect()` because we want the event to
    // be delivered after and not before we are out of this function.
    bufferevent_setcb(bev->get(), nullptr, nullptr, mk_libevent_connect, pcb);
    logger->debug("connect() in progress...");
}

} // namespace libevent
} // namespace mk
#endif

static inline void mk_libevent_connect(bufferevent *, short what, void *p) {
    auto pp = static_cast<std::function<void(short)> *>(p);
    std::function<void(short)> func;
    std::swap(func, *pp);
    delete pp;
    func(what);
}
