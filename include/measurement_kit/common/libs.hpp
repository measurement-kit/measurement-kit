// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_LIBS_HPP
#define MEASUREMENT_KIT_COMMON_LIBS_HPP

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/util.h>

#include <arpa/inet.h>

#include <functional>

struct bufferevent;
struct evbuffer;
struct evdns_base;
struct evdns_request;
struct event;
struct event_base;
struct timeval;

namespace measurement_kit {
namespace common {

struct Libs {

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

    std::function<int(event_base *, int)> event_base_loop = ::event_base_loop;

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

    // Get global libs object
    static Libs *global() {
        static Libs singleton;
        return &singleton;
    }
};

} // namespace common
} // namespace measurement_kit
#endif
