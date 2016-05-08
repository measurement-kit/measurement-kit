// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/connect.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <event2/bufferevent_ssl.h>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/net/error.hpp>
#include <stddef.h>
#include <string.h>

void mk_bufferevent_on_event(bufferevent *bev, short what, void *ptr) {
    auto cb = static_cast<mk::Callback<bufferevent *> *>(ptr);
    if ((what & BEV_EVENT_CONNECTED) != 0) {
        (*cb)(mk::NoError(), bev);
    } else if ((what & BEV_EVENT_TIMEOUT) != 0) {
        (*cb)(mk::net::TimeoutError(), bev);
    } else {
        // TODO: here we should map to the actual error that occurred
        (*cb)(mk::net::NetworkError(), bev);
    }
    delete cb;
}

namespace mk {
namespace net {

void connect_first_of(std::vector<std::string> addresses, int port,
        ConnectFirstOfCb cb, Settings settings, Poller *poller, Var<Logger> logger,
        size_t index, Var<std::vector<Error>> errors) {
    logger->debug("connect_first_of begin");
    if (!errors) {
        errors.reset(new std::vector<Error>());
    }
    if (index >= addresses.size()) {
        logger->debug("connect_first_of all addresses failed");
        cb(*errors, nullptr);
        return;
    }
    double timeout = settings.get("net/timeout", 30.0);
    connect_base(addresses[index], port,
            [=](Error err, bufferevent *bev) {
                errors->push_back(err);
                if (err) {
                    logger->debug("connect_first_of failure");
                    connect_first_of(addresses, port, cb, settings, poller,
                            logger, index + 1, errors);
                    return;
                }
                logger->debug("connect_first_of success");
                cb(*errors, bev);
            },
            timeout, poller, logger);
}

void resolve_hostname(
        std::string hostname, ResolveHostnameCb cb, 
        Settings settings, Poller *poller, Var<Logger> logger) {

    logger->debug("resolve_hostname: %s", hostname.c_str());

    sockaddr_storage storage;
    Var<ResolveHostnameResult> result(new ResolveHostnameResult);

    // If address is a valid IPv4 address, connect directly
    memset(&storage, 0, sizeof storage);
    if (inet_pton(PF_INET, hostname.c_str(), &storage) == 1) {
        logger->debug("resolve_hostname: is valid ipv4");
        result->addresses.push_back(hostname);
        result->inet_pton_ipv4 = true;
        cb(*result);
        return;
    }

    // If address is a valid IPv6 address, connect directly
    memset(&storage, 0, sizeof storage);
    if (inet_pton(PF_INET6, hostname.c_str(), &storage) == 1) {
        logger->debug("resolve_hostname: is valid ipv6");
        result->addresses.push_back(hostname);
        result->inet_pton_ipv6 = true;
        cb(*result);
        return;
    }

    logger->debug("resolve_hostname: ipv4...");

    dns::query("IN", "A", hostname, [=](Error err, dns::Message resp) {
        logger->debug("resolve_hostname: ipv4... done");
        result->ipv4_err = err;
        result->ipv4_reply = resp;
        if (!err) {
            for (dns::Answer answer : resp.answers) {
                result->addresses.push_back(answer.ipv4);
            }
        }
        logger->debug("resolve_hostname: ipv6...");
        dns::query("IN", "AAAA", hostname, [=](Error err, dns::Message resp) {
            logger->debug("resolve_hostname: ipv6... done");
            result->ipv6_err = err;
            result->ipv6_reply = resp;
            if (!err) {
                for (dns::Answer answer : resp.answers) {
                    result->addresses.push_back(answer.ipv6);
                }
            }
            cb(*result);
        }, settings, poller);
    }, settings, poller);
}

void connect_logic(std::string hostname, int port, Callback<Var<ConnectResult>> cb,
        Settings settings, Poller *poller, Var<Logger> logger) {

    Var<ConnectResult> result(new ConnectResult);
    resolve_hostname(hostname,
            [=](ResolveHostnameResult r) {

                result->resolve_result = r;
                if (result->resolve_result.addresses.size() <= 0) {
                    cb(DnsGenericError(), result);
                    return;
                }

                connect_first_of(result->resolve_result.addresses, port,
                        [=](std::vector<Error> e, bufferevent *b) {
                            result->connect_result = e;
                            result->connected_bev = b;
                            if (!b) {
                                cb(ConnectFailedError(), result);
                                return;
                            }
                            cb(NoError(), result);
                        },
                        settings, poller, logger);

            },
            settings ,poller, logger);
}

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl,
                 std::string hostname,
                 Callback<bufferevent *> cb,
                 Poller *poller, Var<Logger> logger) {
    logger->debug("connect ssl...");

    auto bev = bufferevent_openssl_filter_new(poller->get_event_base(),
            orig_bev, ssl, BUFFEREVENT_SSL_CONNECTING,
            BEV_OPT_CLOSE_ON_FREE);
    if (bev == nullptr) {
        bufferevent_free(orig_bev);
        cb(GenericError(), nullptr);
        return;
    }

    bufferevent_setcb(bev, nullptr, nullptr, mk_bufferevent_on_event,
            new Callback<bufferevent *>([cb, logger, hostname](Error err, bufferevent *bev) {
                int hostname_validate_err = 0;

                logger->debug("connect ssl... callback");
                ssl_st *ssl = bufferevent_openssl_get_ssl(bev);

                if (err) {
                    logger->debug("error in connection.");
                    if (err == mk::net::NetworkError()) {
                        long ssl_err = bufferevent_get_openssl_error(bev);
                        err = SSLError(ERR_error_string(ssl_err, NULL));
                    }
                    bufferevent_free(bev);
                    cb(err, nullptr);
                    return;
                }

                long verify_err = SSL_get_verify_result(ssl);
                if (verify_err != X509_V_OK) {
                    debug("ssl: got an invalid certificate");
                    bufferevent_free(bev);
                    cb(SSLInvalidCertificateError(X509_verify_cert_error_string(verify_err)), nullptr);
                    return;
                }

                X509 *server_cert = SSL_get_peer_certificate(ssl);
                if (server_cert == NULL) {
                    logger->debug("ssl: got no certificate");
                    bufferevent_free(bev);
                    cb(SSLNoCertificateError(), nullptr);
                    return;
                }

                hostname_validate_err = tls_check_name(NULL, server_cert, hostname.c_str());
                X509_free(server_cert);
                if (hostname_validate_err != 0) {
                    logger->debug("ssl: got invalid hostname");
                    bufferevent_free(bev);
                    cb(SSLInvalidHostnameError(), nullptr);
                    return;
                }

                cb(err, bev);
            }));
}

} // namespace net
} // namespace mk
