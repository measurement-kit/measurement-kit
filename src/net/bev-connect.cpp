// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/util.h>
#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/dns/response.hpp>
#include <measurement_kit/net/error.hpp>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <vector>
#include "src/common/utils.hpp"
#include "src/dns/getaddrinfo.hpp"
#include "src/net/bev-connect.hpp"
#include "src/net/bev-wrappers.hpp"
#include "src/net/ssl-wrappers.hpp"

// Forward declaration
struct ssl_st;

extern "C" {

static void connect_cb(bufferevent *, short what, void *opaque) {
    auto func = static_cast<std::function<void(short)> *>(opaque);
    (*func)(what);
    delete func;
}
}
namespace mk {
namespace net {

template <class ErrorType = SocketError>
static void setup_cb(Var<bufferevent> bev, OnConnect cb, double timeout,
                     Poller *poller, Logger *logger) {
    timeval tv, *tvp = timeval_init(&tv, timeout);
    mk::net::bufferevent_set_timeouts(bev, tvp, tvp);
    ::bufferevent_setcb(bev.get(), nullptr, nullptr, connect_cb,
                        new std::function<void(short)>(
                            [bev, cb, poller, logger](short what) {
        ::bufferevent_setcb(bev.get(), nullptr, nullptr, nullptr, nullptr);
        mk::net::bufferevent_set_timeouts(bev, nullptr, nullptr);
        logger->debug("result: %s", mk::net::strerror(bev, what).c_str());
        if (what & BEV_EVENT_CONNECTED) {
            cb(NoError(), bev);
        } else {
            cb(ErrorType(), nullptr);
        }
        // Do not let unused objects die immediately because we have been
        // called by C code which is still using them
        if (bev.unique()) {
            logger->debug("garbage collecting unused bev=%p...", bev.get());
            poller->call_soon([bev, logger]() {
                logger->debug("garbage collecting bev=%p... done", bev.get());
            });
        }
    }));
}

void connect(sockaddr *sa, int len, OnConnect cb, double timeout,
             Poller *poller, Logger *logger) {
    Var<bufferevent> bev = mk::net::bufferevent_socket_new(
        poller->get_event_base(), -1, BEV_OPT_CLOSE_ON_FREE);
    Error error = mk::net::bufferevent_socket_connect(bev, sa, len);
    if (error) {
        cb(error, nullptr);
        return;
    }
    setup_cb(bev, cb, timeout, poller, logger);
}

void connect(std::string endpoint, OnConnect cb, double timeout, Poller *poller,
             Logger *logger) {
    logger->debug("connect endpoint=%s", endpoint.data());
    sockaddr_storage stor;
    int len = sizeof(stor);
    sockaddr *sa = (sockaddr *)&stor;
    if (evutil_parse_sockaddr_port(endpoint.c_str(), sa, &len) != 0) {
        cb(EvutilParseSockaddrPortError(), nullptr);
        return;
    }
    connect(sa, len, cb, timeout, poller);
}

static void connect_next(std::vector<std::string> epnts, OnConnect cb,
                         double t, Poller *poller, Logger *log, size_t idx) {
    log->debug("connect_next index=%llu", (unsigned long long)idx);
    if (idx >= epnts.size()) {
        cb(GenericError(), nullptr);
        return;
    }
    connect(epnts[idx],
            [epnts, cb, t, poller, log, idx](Error err, Var<bufferevent> bev) {
                if (!err) {
                    cb(NoError(), bev);
                    return;
                }
                poller->call_soon([epnts, cb, t, poller, log, idx]() {
                    connect_next(epnts, cb, t, poller, log, idx + 1);
                });
            },
            t, poller);
}

void connect_one_of(std::vector<std::string> epnts, OnConnect cb,
                    double timeout, Poller *poller, Logger *logger) {
    logger->debug("connect_one_of...");
    connect_next(epnts, cb, timeout, poller, logger, 0);
}

Error connect_dns_sync(std::string domain, std::string port, OnConnect cb,
                       double timeout, Poller *poller, Logger *logger) {
    auto response = dns::getaddrinfo_query("IN", "A", domain);
    if (!response) {
        return GenericError();
    }
    logger->debug("- %s", domain.c_str());
    std::vector<std::string> endpoints;
    for (auto &str : response.as_value().results) {
        endpoints.push_back(str + ":" + port);
        logger->debug("    - %s", str.c_str());
    }
    connect_one_of(endpoints, cb, timeout, poller, logger);
    return NoError();
}

Error connect_dns_sync(std::string domain, int port, OnConnect cb,
                       double timeout, Poller *poller, Logger *logger) {
    return connect_dns_sync(domain, std::to_string(port), cb, timeout,
                            poller, logger);
}

void connect_ssl(Var<bufferevent> bev, OnConnect cb, double timeout,
                 Poller *poller, Logger *logger) {
    logger->debug("connect_ssl...");
    ssl_st *ssl = WrapperSsl::get_client_ssl();
    bev = mk::net::bufferevent_openssl_filter_new(
        poller->get_event_base(), bev, ssl, BUFFEREVENT_SSL_CONNECTING,
        BEV_OPT_CLOSE_ON_FREE);
    setup_cb(bev, cb, timeout, poller, logger);
}

} // namespace net
} // namespace mk
