// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/connect.hpp"
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
        bufferevent_free(bev);
        (*cb)(mk::net::TimeoutError(), nullptr);
    } else {
        bufferevent_free(bev);
        // TODO: here we should map to the actual error that occurred
        (*cb)(mk::net::NetworkError(), nullptr);
    }
    delete cb;
}

namespace mk {
namespace net {

void connect_first_of(std::vector<std::string> addresses, int port,
        ConnectFirstOfCb cb, double timeout, Poller *poller, Logger *logger,
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
    connect_base(addresses[index], port,
            [=](Error err, bufferevent *bev) {
                errors->push_back(err);
                if (err) {
                    logger->debug("connect_first_of failure");
                    connect_first_of(addresses, port, cb, timeout, poller,
                            logger, index + 1, errors);
                    return;
                }
                logger->debug("connect_first_of success");
                cb(*errors, bev);
            },
            timeout, poller, logger);
}

void resolve_hostname(
        std::string hostname, ResolveHostnameCb cb, Poller *poller,
        Logger *logger) {

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
        }, {}, poller);
    }, {}, poller);
}

void connect(std::string hostname, int port, ConnectCb cb, double timeo,
        Poller *poller, Logger *logger) {

    Var<ConnectResult> result(new ConnectResult);
    resolve_hostname(hostname,
            [=](ResolveHostnameResult r) {

                result->resolve_result = r;
                if (result->resolve_result.addresses.size() <= 0) {
                    result->overall_error = DnsGenericError();
                    cb(*result);
                    return;
                }

                connect_first_of(result->resolve_result.addresses, port,
                        [=](std::vector<Error> e, bufferevent *b) {
                            if (!b) {
                                result->overall_error = ConnectFailedError();
                            }
                            result->connect_result = e;
                            result->connected_bev = b;
                            cb(*result);
                        },
                        timeo, poller, logger);

            },
            poller, logger);
}

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl,
                 Callback<bufferevent *> cb,
                 Poller *poller, Logger *logger) {
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
            new Callback<bufferevent *>([cb, logger](Error err, bufferevent *bev) {
                logger->debug("connect ssl... callback");
                cb(err, bev);
            }));
}

} // namespace net
} // namespace mk
