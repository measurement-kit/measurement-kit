// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECT_HPP
#define SRC_NET_CONNECT_HPP

#include "src/ext/tls_verify.h"

#include "event2/util.h"
#include "src/common/utils.hpp"
#include <arpa/inet.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/net.hpp>
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
void connect_base(std::string address, int port, Callback<Error, bufferevent *> cb,
        double timeout = 10.0, Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global()) {
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
    if ((bev = bufferevent_socket_new(reactor->get_event_base(), -1,
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
            new Callback<Error, bufferevent *>([cb](Error err, bufferevent *bev) {
                if (err) {
                    bufferevent_free(bev);
                    cb(err, nullptr);
                    return;
                }
                cb(err, bev);
            }));
}

typedef std::function<void(std::vector<Error>, bufferevent *)> ConnectFirstOfCb;

void connect_first_of(std::vector<std::string> addresses, int port,
        ConnectFirstOfCb cb, Settings settings = {},
        Var<Reactor> reactor = Reactor::global(), Var<Logger> logger = Logger::global(),
        size_t index = 0, Var<std::vector<Error>> errors = nullptr);

typedef std::function<void(ResolveHostnameResult)> ResolveHostnameCb;

void resolve_hostname(std::string hostname, ResolveHostnameCb cb,
        Settings settings = {}, Var<Reactor> reactor = Reactor::global(), Var<Logger> logger = Logger::global());

void connect_logic(std::string hostname, int port, Callback<Error, Var<ConnectResult>> cb,
        Settings settings = {}, Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

void connect_ssl(bufferevent *orig_bev, ssl_st *ssl,
                 std::string hostname,
                 Callback<Error, bufferevent *> cb,
                 Var<Reactor> = Reactor::global(),
                 Var<Logger> = Logger::global());

} // namespace mk
} // namespace net
#endif
