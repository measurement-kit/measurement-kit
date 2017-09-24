// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_LIBEVENT_CONNECT_HPP
#define PRIVATE_NET_LIBEVENT_CONNECT_HPP

/// \file private/net/libevent_connect.hpp
/// Implementation of connect based on libevent

/*-
                                     _
      ___ ___  _ __  _ __   ___  ___| |_
     / __/ _ \| '_ \| '_ \ / _ \/ __| __|
    | (_| (_) | | | | | | |  __/ (__| |_
     \___\___/|_| |_|_| |_|\___|\___|\__|

    Implementation of connect() based on libevent.
*/

#include "private/common/mock.hpp"           // for MK_MOCK
#include "private/common/utils.hpp"          // for mk::time_now
#include "private/libevent/connection.hpp"   // for mk::libevent::Connection
#include "private/net/emitter.hpp"           // for mk::net::Emitter
#include "private/net/utils.hpp"             // for mk::net::Endpoint
#include <cassert>                           // for assert
#include <cerrno>                            // for errno
#include <event2/bufferevent.h>              // for bufferevent
#include <event2/bufferevent_ssl.h>          // for bufferevent_ssl
#include <measurement_kit/net/transport.hpp> // for mk::Transport
#include <openssl/ssl.h>                     // for SSL

extern "C" {
/// \brief Callback invoked by C code.
static inline void mk_net_libevent_connect_cb(bufferevent *, short, void *);
}

namespace mk {
namespace net {

/*-
          _   _ _
     _  _| |_(_) |___
    | || |  _| | (_-<
     \_,_|\__|_|_/__/

    Common utility functions.
*/

/// \brief Proxy for make_sockaddr because make_sockaddr is overloaded
static Error make_sockaddr_proxy_(std::string s, std::string p,
                                  sockaddr_storage *ss, socklen_t *len) {
    return make_sockaddr(s, p, ss, len);
}

/// \brief Returns proper flags for constructing buffervent.
///
/// Rationale for deferring callbacks
/// ---------------------------------
///
/// When using IOCP on Windows, the kernel calls callbacks when selected
/// events occur (i.e., there is no loop that guarantees callbacks run in
/// the same thread); set DEFER_CALLBACKS to tell libevent to serialize
/// bufferevent's callbacks into the event loop to avoid creating MT issues
/// in code that otherwise (on Unices) is single threaded.
///
/// Yes, the current implementation forces serializing the callbacks also
/// on Unix where this wouldn't be needed thus adding some overhead. For
/// uniformity, I am for serializing for all platforms and then, if we see
/// that there's too much overhead, to only enable that on Windows.
static inline constexpr short bev_flags_() {
    return BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;
}

/// \brief Map bufferevent flags to MK error.
/// \param flags Bufferevent flags. Only the TIMEOUT, ERROR, and EOF flags
/// are inspected to determine the type of error.
/// \param default_error Default error to be used if errno is zero. This
/// is typically indicative of an higher layer error.
/// \return An MK error or NoError() in case no error occurred.
static inline Error map_error_(short flags, Error default_error) {
    assert(default_error != NoError());
    Error err;
    if ((flags & BEV_EVENT_TIMEOUT) != 0) {
        err = TimeoutError();
    } else if ((flags & BEV_EVENT_ERROR) != 0) {
        err = mk::net::map_errno(errno);
        if (err == NoError()) {
            err = default_error; // We must report an error!
        }
    } else if ((flags & BEV_EVENT_EOF) != 0) {
        err = EofError();
    } else {
        /* NOTHING */;
    }
    return err;
}

/*-
     _
    | |_ __ _ __
    |  _/ _| '_ \
     \__\__| .__/
           |_|

    Code to establish a TCP connection.
*/

/// \brief Connect to the remote endpoint.
/// \param address Address of the remote endpoint.
/// \param port Port of the remote endpoint.
/// \param timeout Timeout for connect and subsequent I/O operations.
/// \param reactor Reactor to use.
/// \param logger Logger to use.
/// \param cb Callback called when complete. The first argument will be the
/// error that occurred on failure, or NoError(). The second argument is the
/// connected socket on success, an unconnected but still not `nullptr`
/// socket on failure.
/// \throw GenericError if specific API that should not fail fails.
/// \note In case of early error, the callback may be called immediately.
template <MK_MOCK(make_sockaddr_proxy_), MK_MOCK(bufferevent_socket_new),
          MK_MOCK(bufferevent_set_timeouts),
          MK_MOCK(bufferevent_socket_connect)>
void libevent_connect(std::string address, uint16_t port, double timeout,
                      SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
                      Callback<Error, SharedPtr<Transport>> &&cb) {

    std::string endpoint = [&]() {
        Endpoint endpoint;
        endpoint.hostname = address;
        endpoint.port = port;
        return serialize_endpoint(endpoint);
    }();
    logger->debug("requested connect to %s", endpoint.c_str());

    std::string port_string = std::to_string(port);
    sockaddr_storage storage = {};
    socklen_t salen = 0;
    Error err = make_sockaddr_proxy_(address, port_string, &storage, &salen);
    if (err != NoError()) {
        logger->warn("cannot parse sockaddr from: '%s'", endpoint.c_str());
        cb(err, Emitter::make(reactor, logger));
        return;
    }
    sockaddr *saddr = (sockaddr *)&storage;

    bufferevent *bev;
    if ((bev = bufferevent_socket_new(reactor->get_event_base(), -1,
                                      bev_flags_())) == nullptr) {
        throw GenericError(); // This should not happen
    }

    // Note: this timeout is going to be used both for connect and for
    // subsequent I/O operations (i.e. reads and writes).
    timeval tv, *tvp = timeval_init(&tv, timeout);
    if (bufferevent_set_timeouts(bev, tvp, tvp) != 0) {
        bufferevent_free(bev);
        throw GenericError(); // This should not happen
    }

    double begin = mk::time_now(); // Start measuring connect time

    if (bufferevent_socket_connect(bev, saddr, salen) != 0) {
        logger->warn("connect() for %s failed immediately", endpoint.c_str());
        bufferevent_free(bev);
        Error err = map_error_(BEV_EVENT_ERROR, GenericError());
        logger->warn("reason why connect() has failed: %s", err.what());
        cb(err, Emitter::make(reactor, logger));
        return;
    }

    logger->debug("connect() in progress...");

    // WARNING: set callbacks after connect() otherwise we free `bev` twice
    // NOTE: In case of `new` failure we let the stack unwind
    bufferevent_setcb(
        bev, nullptr, nullptr, mk_net_libevent_connect_cb,
        new Callback<short>([=](short flags) {
            // Reset the bufferevent callback
            bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
            Error err = map_error_(flags, GenericError());
            if (err) {
                logger->warn("connect() for %s failed in its callback: %s",
                             endpoint.c_str(), err.what());
                bufferevent_free(bev);
                cb(err, Emitter::make(reactor, logger));
                return;
            }
            double elapsed = mk::time_now() - begin;
            logger->debug("connect time: %f", elapsed);
            SharedPtr<Transport> txp = libevent::Connection::make(
                    bev, reactor, logger);
            txp->set_connect_time_(elapsed);
            err = disable_nagle(bufferevent_getfd(bev));
            if (err) {
                logger->warn("connect: cannot disable nagle for socket");
            }
            cb(err, txp);
        }));
}

/*-
           _    __ _ _ _
     _____| |  / _(_) | |_ ___ _ _
    (_-<_-< | |  _| | |  _/ -_) '_|
    /__/__/_| |_| |_|_|\__\___|_|

    Code to wrap an existing connection with an SSL filter.
*/

/// \brief Attach bufferevent SSL filter to existing connection.
/// \param txp A connected transport.
/// \param ssl SSL socket to use.
/// \param reactor Reactor to use.
/// \param logger Logger to use.
/// \param cb Callback called when done. The Error argument is NoError()
/// in case of success and an error on failure.
/// \throw GenericError if some API that should not fail fails.
/// \throw std::runtime_error if it is not possible to cast the transport
/// passed as first argument into a transport managed by libevent.
/// \note In case of early error, the callback may be called immediately.
/// \note Whether this call succeeds or not, the \p txp transport
/// passed as first argument will be modified to contain a SSL
/// bufferevent rather than a socket bufferevent. This will not
/// lead to leaks because the SSL bufferevent will reference
/// the socket bufferevent and manage its lifecycle.
template <MK_MOCK(bufferevent_openssl_filter_new)>
void libevent_ssl_connect_filter(SharedPtr<Transport> txp, SSL *ssl,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error> &&callback) {
    // Note: if the cast is not possible a runtime exception will follow
    auto conn = txp.as<libevent::Connection>();
    auto orig_bev = conn->bufferevent_();
    logger->debug("ssl: handshake...");
    auto bev = bufferevent_openssl_filter_new(reactor->get_event_base(),
            orig_bev, ssl, BUFFEREVENT_SSL_CONNECTING, bev_flags_());
    if (bev == nullptr) {
        throw GenericError(); // This should not happen
    }
    bufferevent_setcb(bev, nullptr, nullptr, mk_net_libevent_connect_cb,
            new Callback<short>([=](short flags) {
                // Reset the bufferevent callback
                bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
                Error err = map_error_(flags, GenericError());
                if (err == GenericError()) {
                    long sslerr = bufferevent_get_openssl_error(bev);
                    if (sslerr) {
                        err = SslError(ERR_error_string(sslerr, nullptr));
                    }
                }
                // At this point we have already wrapped the underlying
                // bufferevent, so it would be wrong to destroy `bev` since
                // that would lead to destroying twice the underlying
                // bufferevent resulting in assertion or worst. Instead,
                // just replace the bufferevent in the transport and
                // proceed in reporting the error to the caller.
                conn->set_bufferevent_(bev);
                callback(err);
            }));
}


} // namespace mk
} // namespace net

/*-
             _ _ _             _
     __ __ _| | | |__  __ _ __| |__ ___
    / _/ _` | | | '_ \/ _` / _| / /(_-<
    \__\__,_|_|_|_.__/\__,_\__|_\_\/__/

    C linkage callbacks.
*/

static inline void mk_net_libevent_connect_cb(
        bufferevent *, short flags, void *opaque) {
    auto pcallback = static_cast<mk::Callback<short> *>(opaque);
    mk::Callback<short> callback;
    std::swap(callback, *pcallback);
    delete pcallback;
    // Note: an exception raised by this callback will unwind the stack
    // and tear down the libevent loop leaving it unusable.
    callback(flags);
}
#endif
