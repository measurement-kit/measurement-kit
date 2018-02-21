// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef HAVE_CONFIG_H
#include "config.h" // For MK_CA_BUNDLE
#endif
#ifndef MK_CA_BUNDLE
#error "MK_CA_BUNDLE is not set."
#endif

#include "src/libmeasurement_kit/net/connect_impl.hpp"
#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/net/socks5.hpp"
#include "src/libmeasurement_kit/net/utils.hpp"

#include "src/libmeasurement_kit/net/libevent_emitter.hpp"
#include "src/libmeasurement_kit/net/libssl.hpp"

#include "src/libmeasurement_kit/dns/resolve_hostname.hpp"
#include "src/libmeasurement_kit/dns/query.hpp"
#include "src/libmeasurement_kit/net/connect.hpp"

#include <event2/bufferevent_ssl.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cerrno>
#include <cassert>
#include <cstddef>
#include <cstring>

void mk_bufferevent_on_event(bufferevent *bev, short what, void *ptr) {
    auto cb = static_cast<mk::Callback<mk::Error, bufferevent *> *>(ptr);
    if ((what & BEV_EVENT_CONNECTED) != 0) {
        (*cb)(mk::NoError(), bev);
    } else if ((what & BEV_EVENT_TIMEOUT) != 0) {
        (*cb)(mk::net::TimeoutError(), bev);
    } else {
        mk::Error err;
        /*
         * If there's not network error, assume it's going to be a SSL error,
         * which is reasonable because currently we only use this function
         * as callback for either socket or SSL connect.
         */
        if (errno != 0) {
            err = mk::net::map_errno(errno);
        } else {
            long ssl_err = bufferevent_get_openssl_error(bev);
            std::string s = ERR_error_string(ssl_err, nullptr);
            err = mk::net::SslError(s);
        }
        (*cb)(err, bev);
    }
    delete cb;
}

namespace mk {
namespace net {

void connect_first_of(SharedPtr<ConnectResult> result, int port,
                      ConnectFirstOfCb cb, Settings settings,
                      SharedPtr<Reactor> reactor, SharedPtr<Logger> logger, size_t index,
                      SharedPtr<std::vector<Error>> errors) {
    logger->debug2("connect_first_of begin");
    if (!errors) {
        errors.reset(new std::vector<Error>());
    }
    if (index >= result->resolve_result.addresses.size()) {
        logger->debug2("connect_first_of all addresses failed");
        cb(*errors, nullptr);
        return;
    }
    double timeout = settings.get("net/timeout", 30.0);
    connect_base(result->resolve_result.addresses[index], port,
                 timeout, reactor, logger,
                 [=](Error err, bufferevent *bev, double connect_time) {
                     errors->push_back(err);
                     if (err) {
                         logger->debug2("connect_first_of failure");
                         connect_first_of(result, port, cb, settings,
                                          reactor, logger, index + 1, errors);
                         return;
                     }
                     logger->debug2("connect_first_of success");
                     result->connect_time = connect_time;
                     cb(*errors, bev);
                 });
}

void connect_logic(std::string hostname, int port,
                   Callback<Error, SharedPtr<ConnectResult>> cb, Settings settings,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    SharedPtr<ConnectResult> result(new ConnectResult);
    dns::resolve_hostname(hostname,
                     [=](dns::ResolveHostnameResult r) {

                         result->resolve_result = r;
                         if (result->resolve_result.addresses.size() <= 0) {
                             cb(DnsGenericError(), result);
                             return;
                         }

                         connect_first_of(
                             result, port,
                             [=](std::vector<Error> e, bufferevent *b) {
                                 result->connect_result = e;
                                 result->connected_bev = b;
                                 if (!b) {
                                     if (e.size() == 1) {
                                         // Improvement: do not hide the reason
                                         // why we failed if we have just one
                                         // connect() attempt in the vector
                                         cb(e[0], result);
                                         return;
                                     }
                                     // Otherwise, report them all
                                     Error connect_error = ConnectFailedError();
                                     for (auto se: e) {
                                         connect_error.add_child_error(std::move(se));
                                     }
                                     cb(connect_error, result);
                                     return;
                                 }
                                 Error nagle_error = disable_nagle(
                                    bufferevent_getfd(result->connected_bev)
                                 );
                                 for (auto se: e) {
                                    nagle_error.add_child_error(std::move(se));
                                 }
                                 cb(nagle_error, result);
                             },
                             settings, reactor, logger);

                     },
                     settings, reactor, logger);
}

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl, std::string hostname,
                 Callback<Error, bufferevent *> cb, SharedPtr<Reactor> reactor,
                 SharedPtr<Logger> logger) {
    logger->debug("ssl: handshake...");

    // See similar comment in connect_impl.hpp for rationale.
    static const int flags = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;

    auto bev = bufferevent_openssl_filter_new(
        reactor->get_event_base(), orig_bev, ssl, BUFFEREVENT_SSL_CONNECTING,
        flags);
    if (bev == nullptr) {
        bufferevent_free(orig_bev);
        cb(GenericError(), nullptr);
        return;
    }

    bufferevent_setcb(
        bev, nullptr, nullptr, mk_bufferevent_on_event,
        new Callback<Error, bufferevent *>(
            [cb, logger, hostname](Error err, bufferevent *bev) {
                ssl_st *ssl = bufferevent_openssl_get_ssl(bev);

                if (err) {
                    logger->debug("ssl: handshake error: %s", err.what());
                    bufferevent_free(bev);
                    cb(err, nullptr);
                    return;
                }

                err = libssl::verify_peer(hostname, ssl, logger);
                if (err) {
                    bufferevent_free(bev);
                    cb(err, nullptr);
                    return;
                }

                logger->debug("ssl: handshake... complete");
                cb(err, bev);
            }));
}

void connect_many(std::string address, int port, int num,
                  ConnectManyCb callback, Settings settings,
                  SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    connect_many_impl<net::connect>(connect_many_make(
        address, port, num, callback, settings, reactor, logger));
}

void connect(std::string address, int port,
             Callback<Error, SharedPtr<Transport>> callback, Settings settings,
             SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    if (settings.find("net/dumb_transport") != settings.end()) {
        callback(NoError(), make_txp<Emitter>(
            0.0, nullptr, reactor, logger));
        return;
    }
    if (settings.find("net/socks5_proxy") != settings.end()) {
        socks5_connect(address, port, settings, callback, reactor, logger);
        return;
    }
    if (settings.find("net/timeout") == settings.end()) {
        settings["net/timeout"] = 30.0;
    }
    double timeout = settings["net/timeout"].as<double>();
    connect_logic(
        address, port,
        [=](Error err, SharedPtr<ConnectResult> r) {
            if (err) {
                callback(err, make_txp<Emitter>(
                    timeout, r, reactor, logger));
                return;
            }
            if (settings.find("net/ssl") != settings.end()) {
                std::string cbp = MK_CA_BUNDLE;
                if (settings.find("net/ca_bundle_path") != settings.end()) {
                    cbp = settings.at("net/ca_bundle_path");
                }
                ErrorOr<SSL *> cssl = libssl::Cache<>::thread_local_instance()
                    ->get_client_ssl(cbp, address, logger);
                if (!cssl) {
                    Error err = cssl.as_error();
                    bufferevent_free(r->connected_bev);
                    callback(err, make_txp<Emitter>(
                        timeout, r, reactor, logger));
                    return;
                }
                ErrorOr<bool> allow_ssl23 =
                    settings.get_noexcept("net/allow_ssl23", false);
                if (!allow_ssl23) {
                    Error err = ValueError();
                    bufferevent_free(r->connected_bev);
                    callback(err, make_txp<Emitter>(
                        timeout, r, reactor, logger));
                    return;
                }
                if (*allow_ssl23 == true) {
                    logger->info("Re-enabling SSLv2 and SSLv3");
                    libssl::enable_v23(*cssl);
                }
                connect_ssl(r->connected_bev, *cssl, address,
                            [r, callback, timeout, reactor,
                             logger, settings](Error err, bufferevent *bev) {
                                if (err) {
                                    callback(err, make_txp<Emitter>(
                                            timeout, r, reactor, logger));
                                    return;
                                }
                                ErrorOr<bool> allow_dirty_shutdown =
                                    settings.get_noexcept(
                                        "net/ssl_allow_dirty_shutdown", false);
                                if (!allow_dirty_shutdown) {
                                    Error err = allow_dirty_shutdown.as_error();
                                    bufferevent_free(bev);
                                    callback(err, make_txp<Emitter>(
                                            timeout, r, reactor, logger));
                                    return;
                                }
                                if (*allow_dirty_shutdown == true) {
                                    /*
                                     * This useful libevent function is only
                                     * available since libevent 2.1.0:
                                     */
#ifdef HAVE_BUFFEREVENT_OPENSSL_SET_ALLOW_DIRTY_SHUTDOWN
                                    bufferevent_openssl_set_allow_dirty_shutdown(
                                        bev, 1);
                                    logger->info("Allowing dirty SSL shutdown");
#else
                                    logger->warn("Cannot tell libevent to "
                                                 "allow SSL dirty shutdowns "
                                                 "as requested by the "
                                                 "programmer: as a result"
                                                 "some SSL connections may "
                                                 "interrupt abruptly with an "
                                                 "error. This happens because "
                                                 "you are not using version "
                                                 "2.1.x of libevent.");
#endif
                                }
                                assert(err == NoError());
                                callback(err, make_txp(
                                    net::LibeventEmitter::make(
                                        bev, reactor, logger),
                                            timeout, r));
                            },
                            reactor, logger);
                return;
            }
            assert(err == NoError());
            callback(err, make_txp(net::LibeventEmitter::make(
                r->connected_bev, reactor, logger),
                    timeout, r));
        },
        settings, reactor, logger);
}

} // namespace net
} // namespace mk
