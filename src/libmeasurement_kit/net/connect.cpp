// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef HAVE_CONFIG_H
#include "config.h" // For MK_CA_BUNDLE
#endif
#ifndef MK_CA_BUNDLE
#error "MK_CA_BUNDLE is not set."
#endif

#include "private/net/libevent_connect.hpp"
#include "private/net/connect_impl.hpp"
#include "private/net/emitter.hpp"
#include "private/net/socks5.hpp"
#include "private/net/utils.hpp"
#include "private/net/libssl.hpp"

#include <measurement_kit/dns.hpp>
#include <measurement_kit/net.hpp>

#include <event2/bufferevent_ssl.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cerrno>
#include <cassert>
#include <cstddef>
#include <cstring>

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
        cb(GenericError(), *errors);
        return;
    }
    // Note: this timeout is going to be used both for connect and for
    // subsequent I/O operations (i.e. reads and writes).
    double timeout = settings.get("net/timeout", 30.0);
    libevent_connect(result->resolve_result.addresses[index], port,
                 timeout, reactor, logger,
                 [=](Error err, SharedPtr<Transport> txp) {
                     errors->push_back(err);
                     if (err) {
                         logger->debug2("connect_first_of failure");
                         connect_first_of(result, port, cb, settings,
                                          reactor, logger, index + 1, errors);
                         return;
                     }
                     logger->debug2("connect_first_of success");
                     result->connected_txp = txp;
                     cb(NoError(), *errors);
                 });
}

void connect_logic(std::string hostname, int port,
                   Callback<Error, SharedPtr<ConnectResult>> cb, Settings settings,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    SharedPtr<ConnectResult> result(new ConnectResult);
    result->connected_txp = Emitter::make(reactor, logger);
    dns::resolve_hostname(hostname,
                     [=](dns::ResolveHostnameResult r) {

                         result->resolve_result = r;
                         if (result->resolve_result.addresses.size() <= 0) {
                             cb(DnsGenericError(), result);
                             return;
                         }

                         connect_first_of(
                             result, port,
                             [=](Error overall, std::vector<Error> e) {
                                 result->connected_txp->set_connect_errors_(e);
                                 if (!!overall) {
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
                                         connect_error.add_child_error(se);
                                     }
                                     cb(connect_error, result);
                                     return;
                                 }
                                 cb(NoError(), result);
                             },
                             settings, reactor, logger);

                     },
                     settings, reactor, logger);
}

void connect_ssl(SharedPtr<Transport> txp, ssl_st *ssl, std::string hostname,
                 Callback<Error> cb, SharedPtr<Reactor> reactor,
                 SharedPtr<Logger> logger) {
    logger->debug("ssl: handshake...");
    libevent_ssl_connect_filter(txp, ssl, reactor, logger,
            [=](Error err) {
                if (err) {
                    logger->debug("ssl: handshake error: %s", err.what());
                    cb(err);
                    return;
                }

                err = libssl::verify_peer(hostname, ssl, logger);
                if (err) {
                    cb(err);
                    return;
                }

                logger->debug("ssl: handshake... complete");
                cb(err);
            });
}

void connect_ssl(SharedPtr<Transport> txp, std::string hostname,
        Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
        Callback<Error> &&callback) {
    std::string cbp = MK_CA_BUNDLE;
    if (settings.find("net/ca_bundle_path") != settings.end()) {
        cbp = settings.at("net/ca_bundle_path");
    }
    ErrorOr<SSL *> cssl =
            libssl::Cache<>::thread_local_instance().get_client_ssl(
                    cbp, hostname, logger);
    if (!cssl) {
        Error err = cssl.as_error();
        callback(err);
        return;
    }
    ErrorOr<bool> allow_ssl23 = settings.get_noexcept("net/allow_ssl23", false);
    if (!allow_ssl23) {
        Error err = ValueError();
        callback(err);
        return;
    }
    if (*allow_ssl23 == true) {
        logger->info("Re-enabling SSLv2 and SSLv3");
        libssl::enable_v23(*cssl);
    }
    connect_ssl(txp, *cssl, hostname,
            [=](Error err) {
                if (err) {
                    callback(err);
                    return;
                }
                ErrorOr<bool> allow_dirty_shutdown = settings.get_noexcept(
                        "net/ssl_allow_dirty_shutdown", false);
                if (!allow_dirty_shutdown) {
                    Error err = allow_dirty_shutdown.as_error();
                    callback(err);
                    return;
                }
                if (*allow_dirty_shutdown == true) {
#ifdef HAVE_BUFFEREVENT_OPENSSL_SET_ALLOW_DIRTY_SHUTDOWN
                    // FIXME: this is now broken...
                    // This useful libevent function is only
                    // available since libevent 2.1.0:
                    bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
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
                callback(err);
            },
            reactor, logger);
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
        callback(NoError(), Emitter::make(reactor, logger));
        return;
    }
    if (settings.find("net/socks5_proxy") != settings.end()) {
        socks5_connect(address, port, settings, callback, reactor, logger);
        return;
    }
    connect_logic(
        address, port,
        [=](Error err, SharedPtr<ConnectResult> r) {
            // Whatever transport it is, it know about the resolve result now
            r->connected_txp->set_dns_result_(r->resolve_result);
            if (err) {
                // XXX: perhaps also remove `make_txp`
                // but FIXME we're probably also losing info!!!
                callback(err, r->connected_txp);
                return;
            }
            if (settings.find("net/ssl") != settings.end()) {
                connect_ssl(r->connected_txp, address, settings, reactor,
                        logger,
                        [=](Error err) {
                            callback(err, r->connected_txp);
                        });
                return;
            }
            assert(err == NoError());
            callback(err, r->connected_txp);
        },
        settings, reactor, logger);
}

} // namespace net
} // namespace mk
