// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_LISTEN_HPP
#define SRC_NET_LISTEN_HPP

#include "src/common/utils.hpp"
#include "src/net/connection.hpp"
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <functional>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <sys/socket.h>

extern "C" {

void mk_listener_cb(evconnlistener *sl, evutil_socket_t s, sockaddr *saddr,
        int salen, void *opaque);

} // extern "C"
namespace mk {
namespace net {

typedef std::function<void(bufferevent *)> ListenCb;
typedef std::function<void(evutil_socket_t)> ListenInternalCb;

inline void listen4(std::string address, int port, ListenCb cb) {

    sockaddr_storage storage;
    socklen_t salen;
    if (storage_init(&storage, &salen, PF_INET, address.c_str(), port) != 0) {
        throw ValueError();
    }

    auto cbp = new ListenInternalCb([cb](evutil_socket_t so) {
        bufferevent *bev = bufferevent_socket_new(
                Reactor::global()->get_event_base(), so, BEV_OPT_CLOSE_ON_FREE);
        if (bev == nullptr) {
            (void)evutil_closesocket(so);
            return;
        }
        cb(bev);
    });

    // Note: the listener is never freed and hence Valgrind is probably
    // going to complain that we have a memory leak here.
    evconnlistener *sl = evconnlistener_new_bind(
            Reactor::global()->get_event_base(),
            mk_listener_cb, cbp, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (sockaddr *)&storage, salen);
    if (sl == nullptr) {
        delete cbp;
        throw EvconnlistenerNewBindError();
    }
}

} // namespace mk
} // namespace net
#endif
