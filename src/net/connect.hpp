// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECT_HPP
#define SRC_NET_CONNECT_HPP

#include "event2/util.h"
#include "measurement_kit/common.hpp"
#include "measurement_kit/dns.hpp"
#include "src/common/utils.hpp"
#include <arpa/inet.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <vector>

struct bufferevent;
struct ssl_st;

extern "C" {
void mk_bufferevent_on_event(bufferevent *, short, void *);
}

namespace mk {
namespace net {

template <MK_MOCK(evutil_parse_sockaddr_port), MK_MOCK(bufferevent_socket_new),
        MK_MOCK(bufferevent_set_timeouts), MK_MOCK(bufferevent_socket_connect)>
void connect_base(std::string address, int port, Callback<bufferevent *> cb,
        double timeout = 10.0, Poller *poller = Poller::global(),
        Logger *logger = Logger::global()) {
    logger->debug("connect_base %s:%d", address.c_str(), port);

    std::stringstream ss;
    ss << address << ":" << port;
    std::string endpoint = ss.str();
    sockaddr_storage storage;
    sockaddr *saddr = (sockaddr *)&storage;
    int salen = sizeof storage;

    if (evutil_parse_sockaddr_port(endpoint.c_str(), saddr, &salen) != 0) {
        cb(GenericError(), nullptr);
        return;
    }

    bufferevent *bev;
    if ((bev = bufferevent_socket_new(poller->get_event_base(), -1,
                 BEV_OPT_CLOSE_ON_FREE)) == nullptr) {
        throw GenericError(); // This should not happen
    }

    timeval tv, *tvp = timeval_init(&tv, timeout);
    if (bufferevent_set_timeouts(bev, tvp, tvp) != 0) {
        bufferevent_free(bev);
        throw GenericError(); // This should not happen
    }

    if (bufferevent_socket_connect(bev, saddr, salen) != 0) {
        bufferevent_free(bev);
        cb(GenericError(), nullptr);
        return;
    }

    // WARNING: set callbacks after connect() otherwise we free `bev` twice
    // NOTE: In case of `new` failure we let the stack unwind
    bufferevent_setcb(bev, nullptr, nullptr, mk_bufferevent_on_event,
            new Callback<bufferevent *>(cb));
}

typedef std::function<void(std::vector<Error>, bufferevent *)> ConnectFirstOfCb;

void connect_first_of(std::vector<std::string> addresses, int port,
        ConnectFirstOfCb cb, double timeout = 10.0,
        Poller *poller = Poller::global(), Logger *logger = Logger::global(),
        size_t index = 0, Var<std::vector<Error>> errors = nullptr);

struct ResolveHostnameResult {
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;

    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;

    std::vector<std::string> addresses;
};

typedef std::function<void(ResolveHostnameResult)> ResolveHostnameCb;

void resolve_hostname(std::string hostname, ResolveHostnameCb cb,
        Poller *poller = Poller::global(), Logger *logger = Logger::global());

struct ConnectResult {
    ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    Error overall_error;
    bufferevent *connected_bev;
};

typedef std::function<void(ConnectResult)> ConnectCb;

void connect(std::string hostname, int port, ConnectCb cb, double timeo = 10.0,
        Poller *poller = Poller::global(), Logger *logger = Logger::global());

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl,
                 Callback<bufferevent *> cb,
                 Poller * = Poller::global(),
                 Logger * = Logger::global());

} // namespace mk
} // namespace net
#endif
