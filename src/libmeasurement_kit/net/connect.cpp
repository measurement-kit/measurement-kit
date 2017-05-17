// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef HAVE_CONFIG_H
#include "config.h" // For MK_CA_BUNDLE
#endif

#include "../net/connect_impl.hpp"
#include "../net/emitter.hpp"
#include "../net/socks5.hpp"
#include "../net/utils.hpp"

#include "../libevent/connection.hpp"
#include "../net/ssl_context.hpp"

#include <measurement_kit/dns.hpp>
#include <measurement_kit/net.hpp>

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

using namespace mk::libevent;

void connect_first_of(Var<ConnectResult> result, int port,
                      ConnectFirstOfCb cb, Settings settings,
                      Var<Reactor> reactor, Var<Logger> logger, size_t index,
                      Var<std::vector<Error>> errors) {
    logger->debug("connect_first_of begin");
    if (!errors) {
        errors.reset(new std::vector<Error>());
    }
    if (index >= result->resolve_result.addresses.size()) {
        logger->debug("connect_first_of all addresses failed");
        cb(*errors, nullptr);
        return;
    }
    double timeout = settings.get("net/timeout", 30.0);
    connect_base(result->resolve_result.addresses[index], port,
                 [=](Error err, bufferevent *bev, double connect_time) {
                     errors->push_back(err);
                     if (err) {
                         logger->debug("connect_first_of failure");
                         connect_first_of(result, port, cb, settings,
                                          reactor, logger, index + 1, errors);
                         return;
                     }
                     logger->debug("connect_first_of success");
                     result->connect_time = connect_time;
                     cb(*errors, bev);
                 },
                 timeout, reactor, logger);
}

void connect_logic(std::string hostname, int port,
                   Callback<Error, Var<ConnectResult>> cb, Settings settings,
                   Var<Reactor> reactor, Var<Logger> logger) {

    Var<ConnectResult> result(new ConnectResult);
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
                                         connect_error.add_child_error(se);
                                     }
                                     cb(connect_error, result);
                                     return;
                                 }
                                 Error nagle_error = disable_nagle(
                                    bufferevent_getfd(result->connected_bev)
                                 );
                                 for (auto se: e) {
                                    nagle_error.add_child_error(se);
                                 }
                                 cb(nagle_error, result);
                             },
                             settings, reactor, logger);

                     },
                     settings, reactor, logger);
}

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl, std::string hostname,
                 Callback<Error, bufferevent *> cb, Var<Reactor> reactor,
                 Var<Logger> logger) {
    logger->debug("connect ssl...");

    // Perhaps DEFER not needed on SSL but setting it won't hurt. See
    // the similar comment in connect_impl.hpp for rationale.
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
                int hostname_validate_err = 0;

                logger->debug("connect ssl... callback (error: %d)", err.code);
                ssl_st *ssl = bufferevent_openssl_get_ssl(bev);

                if (err) {
                    std::string s = err.explain();
                    logger->debug("error in connection: %s", s.c_str());
                    bufferevent_free(bev);
                    cb(err, nullptr);
                    return;
                }

                logger->debug("SSL version: %s", SSL_get_version(ssl));

                long verify_err = SSL_get_verify_result(ssl);
                if (verify_err != X509_V_OK) {
                    debug("ssl: got an invalid certificate");
                    bufferevent_free(bev);
                    cb(SslInvalidCertificateError(
                           X509_verify_cert_error_string(verify_err)),
                       nullptr);
                    return;
                }

                X509 *server_cert = SSL_get_peer_certificate(ssl);
                if (server_cert == nullptr) {
                    logger->debug("ssl: got no certificate");
                    bufferevent_free(bev);
                    cb(SslNoCertificateError(), nullptr);
                    return;
                }

                hostname_validate_err =
                    tls_check_name(nullptr, server_cert, hostname.c_str());
                X509_free(server_cert);
                if (hostname_validate_err != 0) {
                    logger->debug("ssl: got invalid hostname");
                    bufferevent_free(bev);
                    cb(SslInvalidHostnameError(), nullptr);
                    return;
                }

                cb(err, bev);
            }));
}

void connect_many(std::string address, int port, int num,
                  ConnectManyCb callback, Settings settings,
                  Var<Reactor> reactor, Var<Logger> logger) {
    connect_many_impl<net::connect>(connect_many_make(
        address, port, num, callback, settings, reactor, logger));
}

void connect(std::string address, int port,
             Callback<Error, Var<Transport>> callback, Settings settings,
             Var<Reactor> reactor, Var<Logger> logger) {
    if (settings.find("net/dumb_transport") != settings.end()) {
        callback(NoError(), Var<Transport>(new Emitter(reactor, logger)));
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
        [=](Error err, Var<ConnectResult> r) {
            if (err) {
                err.context = r;
                callback(err, nullptr);
                return;
            }
            if (settings.find("net/ssl") != settings.end()) {
                std::string cbp
#ifdef MK_CA_BUNDLE
                    = MK_CA_BUNDLE
#endif
                ;
                if (settings.find("net/ca_bundle_path") != settings.end()) {
                    cbp = settings.at("net/ca_bundle_path");
                }
                logger->debug("ca_bundle_path: '%s'", cbp.c_str());
                ErrorOr<Var<SslContext>> ssl_context = SslContext::make(cbp);
                if (!ssl_context) {
                    Error err = ssl_context.as_error();
                    err.context = r;
                    bufferevent_free(r->connected_bev);
                    callback(err, nullptr);
                    return;
                }
                ErrorOr<SSL *> cssl = (*ssl_context)->get_client_ssl(address);
                if (!cssl) {
                    Error err = cssl.as_error();
                    err.context = r;
                    bufferevent_free(r->connected_bev);
                    callback(err, nullptr);
                    return;
                }
                ErrorOr<bool> allow_ssl23 =
                    settings.get_noexcept("net/allow_ssl23", false);
                if (!allow_ssl23) {
                    Error err = ValueError();
                    err.context = r;
                    bufferevent_free(r->connected_bev);
                    callback(err, nullptr);
                    return;
                }
                if (*allow_ssl23 == true) {
                    logger->info("Re-enabling SSLv2 and SSLv3");
                    SSL_clear_options(*cssl, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3);
                }
                connect_ssl(r->connected_bev, *cssl, address,
                            [r, callback, timeout, ssl_context, reactor,
                             logger, settings](Error err, bufferevent *bev) {
                                if (err) {
                                    err.context = r;
                                    callback(err, nullptr);
                                    return;
                                }
                                ErrorOr<bool> allow_dirty_shutdown =
                                    settings.get_noexcept(
                                        "net/ssl_allow_dirty_shutdown", false);
                                if (!allow_dirty_shutdown) {
                                    Error err = allow_dirty_shutdown.as_error();
                                    err.context = r;
                                    bufferevent_free(bev);
                                    callback(err, nullptr);
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
                                Var<Transport> txp =
                                    libevent::Connection::make(
                                        bev, reactor, logger);
                                txp->set_timeout(timeout);
                                assert(err == NoError());
                                err.context = r;
                                callback(err, txp);
                            },
                            reactor, logger);
                return;
            }
            Var<Transport> txp =
                libevent::Connection::make(r->connected_bev, reactor, logger);
            txp->set_timeout(timeout);
            assert(err == NoError());
            err.context = r;
            callback(err, txp);
        },
        settings, reactor, logger);
}

ErrorOr<double> get_connect_time(Error err) {
    Var<ConnectResult> cr = err.context.as<ConnectResult>();
    if (!cr) {
        return GenericError();
    }
    return cr->connect_time;
}

ErrorOr<std::vector<double>> get_connect_times(Error err) {
    Var<ConnectManyResult> cmr = err.context.as<ConnectManyResult>();
    if (!cmr) {
        return GenericError();
    }
    std::vector<double> connect_times;
    for (auto &cr: cmr->results) {
        connect_times.push_back(cr->connect_time);
    }
    return connect_times;
}

ConnectResult::~ConnectResult() {}
ConnectManyResult::~ConnectManyResult() {}

} // namespace net
} // namespace mk
