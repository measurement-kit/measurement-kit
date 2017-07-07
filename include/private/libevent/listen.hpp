// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_LISTEN_HPP
#define PRIVATE_LIBEVENT_LISTEN_HPP

#include "../libevent/connection.hpp"
#include "../net/utils.hpp"

#include <measurement_kit/net.hpp>

#include <event2/listener.h>

extern "C" {

void mk_listener_cb(evconnlistener *sl, evutil_socket_t s, sockaddr *saddr,
        int salen, void *opaque);

} // extern "C"
namespace mk {
namespace libevent {

typedef std::function<void(bufferevent *)> ListenCb;
typedef std::function<void(evutil_socket_t)> ListenInternalCb;

inline void listen4(std::string address, int port, ListenCb cb) {

    sockaddr_storage storage;
    socklen_t salen;
    if (net::storage_init(&storage, &salen, PF_INET, address.c_str(), port,
                          Logger::global()) != 0) {
        throw ValueError();
    }

    auto cbp = new ListenInternalCb([cb](evutil_socket_t so) {
        // See similar comment in connect_impl.hpp for DEFER rationale
        static const int flgs = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS;
        bufferevent *bev = bufferevent_socket_new(
                Reactor::global()->get_event_base(), so, flgs);
        if (bev == nullptr) {
            (void)evutil_closesocket(so);
            return;
        }
        if (net::disable_nagle(so) != NoError()) {
            bufferevent_free(bev);
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
        throw net::EvconnlistenerNewBindError();
    }
}

} // namespace libevent
} // namespace net
#endif
