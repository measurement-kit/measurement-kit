/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_COMMON_LIBEVENT_HPP
#define IGHT_COMMON_LIBEVENT_HPP

//
// Libevent abstraction layer.
//

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/event.h>

#include <arpa/inet.h>

#include <functional>
#include <stdexcept>

#include <ight/common/constraints.hpp>

namespace ight {
namespace common {
namespace libevent {

using namespace ight::common::constraints;

struct Libevent {

    /*
     * bufferevent
     */

    std::function<bufferevent *(event_base *, evutil_socket_t, int)>
        bufferevent_socket_new = ::bufferevent_socket_new;

    std::function<void(bufferevent *)> bufferevent_free = ::bufferevent_free;

    /*
     * event_base
     */

    std::function<event_base *(void)> event_base_new = ::event_base_new;

    std::function<int(event_base*, int)> event_base_loop = ::event_base_loop;

    std::function<int(event_base *)> event_base_dispatch =
        ::event_base_dispatch;

    std::function<int(event_base *)> event_base_loopbreak =
        ::event_base_loopbreak;

    std::function<void(event_base *)> event_base_free = ::event_base_free;

    /*
     * evdns_base
     */

    std::function<evdns_base *(event_base *, int)> evdns_base_new =
        ::evdns_base_new;

    std::function<void(evdns_base *, int)> evdns_base_free = ::evdns_base_free;

    std::function<int(evdns_base *, const char *)>
        evdns_base_nameserver_ip_add = ::evdns_base_nameserver_ip_add;

    std::function<int(evdns_base *, const char *, const char *)>
        evdns_base_set_option = ::evdns_base_set_option;

    std::function<evdns_request *(evdns_base *, const char *, int,
                                  evdns_callback_type, void *)>
        evdns_base_resolve_ipv4 = ::evdns_base_resolve_ipv4;

    std::function<evdns_request *(evdns_base *, const char *, int,
                                  evdns_callback_type, void *)>
        evdns_base_resolve_ipv6 = ::evdns_base_resolve_ipv6;

    std::function<evdns_request *(evdns_base *, const struct in_addr *, int,
                                  evdns_callback_type, void *)>
        evdns_base_resolve_reverse = ::evdns_base_resolve_reverse;

    std::function<evdns_request *(evdns_base *, const struct in6_addr *, int,
                                  evdns_callback_type, void *)>
        evdns_base_resolve_reverse_ipv6 = ::evdns_base_resolve_reverse_ipv6;

    std::function<void(int, char, int, int, void *, void *)> evdns_reply_hook;

    /*
     * event
     */

    std::function<event *(event_base *, evutil_socket_t, short,
                          event_callback_fn, void *)> event_new = ::event_new;

    std::function<int(event *, timeval *)> event_add = ::event_add;

    std::function<int(event *)> event_del = ::event_del;

    std::function<void(event *)> event_free = ::event_free;

    /*
     * evbuffer
     */

    std::function<evbuffer *(void)> evbuffer_new = ::evbuffer_new;

    std::function<void(evbuffer *)> evbuffer_free = ::evbuffer_free;

    //
    // libc functions (we should probably rename this class and this file)
    //

    std::function<int(int, const char *, void *)> inet_pton = ::inet_pton;

    std::function<const char *(int, const void *, char *, socklen_t)>
        inet_ntop = ::inet_ntop;
};

struct GlobalLibevent {

    GlobalLibevent(void) { /* nothing */ }

    static Libevent *get(void) {
        static Libevent singleton;
        return (&singleton);
    }

    GlobalLibevent(GlobalLibevent &) = delete;
    GlobalLibevent &operator=(GlobalLibevent &) = delete;
    GlobalLibevent(GlobalLibevent &&) = delete;
    GlobalLibevent &operator=(GlobalLibevent &&) = delete;
};

class Evbuffer : public NonCopyable, public NonMovable {

    Libevent *libevent = GlobalLibevent::get();
    evbuffer *evbuf = NULL;

  public:
    Evbuffer(Libevent *lev = NULL) {
        if (lev != NULL)
            libevent = lev;
    }

    ~Evbuffer(void) {
        if (evbuf != NULL)
            libevent->evbuffer_free(evbuf);
    }

    operator evbuffer *(void) {
        if (evbuf == NULL && (evbuf = libevent->evbuffer_new()) == NULL)
            throw std::bad_alloc();
        return (evbuf);
    }

    Libevent *get_libevent(void) { return (libevent); }
};

class BuffereventSocket : public NonCopyable, public NonMovable {

    Libevent *libevent = GlobalLibevent::get();
    bufferevent *bev = NULL;

  public:
    BuffereventSocket(Libevent *lev = NULL) {
        if (lev != NULL)
            libevent = lev;
    }

    BuffereventSocket(event_base *base, evutil_socket_t fd, int options,
                      Libevent *lev = NULL) {
        make(base, fd, options, lev);
    }

    void make(event_base *base, evutil_socket_t fd, int options,
              Libevent *lev = NULL) {
        close();
        if (lev == NULL)
            libevent = GlobalLibevent::get();
        else
            libevent = lev;
        if ((bev = libevent->bufferevent_socket_new(base, fd, options)) == NULL)
            throw std::bad_alloc();
    }

    ~BuffereventSocket(void) { close(); }

    void close() {
        if (bev != NULL) {
            libevent->bufferevent_free(bev);
            bev = NULL;
        }
    }

    operator bufferevent *(void) {
        if (bev == NULL)
            throw std::runtime_error("Accessing NULL bufferevent");
        return (bev);
    }

    Libevent *get_libevent(void) { return (libevent); }
};

}}}
#endif
