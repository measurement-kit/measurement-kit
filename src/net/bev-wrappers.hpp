// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_BEV_WRAPPERS_HPP
#define SRC_NET_BEV_WRAPPERS_HPP

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/util.h>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/var.hpp>
#include <stddef.h>
#include <string>
#include "src/common/mock.hpp"

// Forward declarations
struct bufferevent;
struct evbuffer;
struct event_base;
struct sockaddr;
struct ssl_st;
struct timeval;

namespace mk {
namespace net {

std::string event_string(short what);

std::string strerror(Var<bufferevent> bev, short what);

template <MK_MOCK_ALLOC(bufferevent_socket_new, bufferevent_free)>
Var<bufferevent> bufferevent_socket_new(event_base *evbase,
                                        evutil_socket_t sock,
                                        int options) {
    Var<bufferevent> bev(mocked_alloc(evbase, sock, options),
                         [](bufferevent *buffev) {
                             if (buffev != nullptr) {
                                 mocked_dealloc(buffev);
                             }
                         });
    if (!bev) {
        MK_THROW(BuffereventSocketNewError);
    }
    return bev;
}

template <MK_MOCK_ALLOC(bufferevent_openssl_filter_new, bufferevent_free)>
Var<bufferevent> bufferevent_openssl_filter_new(//
        event_base *evbase, Var<bufferevent> orig, ssl_st *ssl,
        enum bufferevent_ssl_state st, int opts) {
    Var<bufferevent> bev(mocked_alloc(evbase, orig.get(), ssl, st, opts),
                         [orig](bufferevent *buffev) {
                             if (buffev != nullptr) {
                                 mocked_dealloc(buffev);
                             }
                         });
    if (!bev) {
        MK_THROW(BuffereventOpensslFilterNewError);
    }
    if (opts & BEV_OPT_CLOSE_ON_FREE) {
        orig.reset(); // Steal pointer
    }
    return bev;
}

template <MK_MOCK(bufferevent_socket_connect)>
Error __attribute__((warn_unused_result))
bufferevent_socket_connect(Var<bufferevent> bev, sockaddr *saddr, int len) {
    if (mocked_func(bev.get(), saddr, len) != 0) {
        return BuffereventSocketConnectError();
    }
    return NoError();
}

template <MK_MOCK(bufferevent_write)>
void bufferevent_write(Var<bufferevent> bev, const void *base, size_t count) {
    if (mocked_func(bev.get(), base, count) != 0) {
        MK_THROW(BuffereventWriteError);
    }
}

template <MK_MOCK(bufferevent_write_buffer)>
void bufferevent_write_buffer(Var<bufferevent> bev, evbuffer *src) {
    if (mocked_func(bev.get(), src) != 0) {
        MK_THROW(BuffereventWriteBufferError);
    }
}

template <MK_MOCK(bufferevent_read)>
size_t bufferevent_read(Var<bufferevent> bev, void *base, size_t count) {
    return mocked_func(bev.get(), base, count);
}

template <MK_MOCK(bufferevent_read_buffer)>
void bufferevent_read_buffer(Var<bufferevent> bev, evbuffer *dest) {
    if (mocked_func(bev.get(), dest) != 0) {
        MK_THROW(BuffereventReadBufferError);
    }
}

template <MK_MOCK(bufferevent_enable)>
void bufferevent_enable(Var<bufferevent> bev, short what) {
    if (mocked_func(bev.get(), what) != 0) {
        MK_THROW(BuffereventEnableError);
    }
}

template <MK_MOCK(bufferevent_disable)>
void bufferevent_disable(Var<bufferevent> bev, short what) {
    if (mocked_func(bev.get(), what) != 0) {
        MK_THROW(BuffereventDisableError);
    }
}

template <MK_MOCK(bufferevent_set_timeouts)>
void bufferevent_set_timeouts(Var<bufferevent> bev, const timeval *rtimeo,
                              const timeval *wtimeo) {
    if (mocked_func(bev.get(), rtimeo, wtimeo) != 0) {
        MK_THROW(BuffereventSetTimeoutsError);
    }
}

template <MK_MOCK(bufferevent_get_openssl_error)>
unsigned long bufferevent_get_openssl_error(Var<bufferevent> bev) {
    return mocked_func(bev.get());
}

template <MK_MOCK(bufferevent_get_input)>
evbuffer *bufferevent_get_input(Var<bufferevent> bev) {
    return mocked_func(bev.get());
}

template <MK_MOCK(bufferevent_get_output)>
evbuffer *bufferevent_get_output(Var<bufferevent> bev) {
    return mocked_func(bev.get());
}

} // namespace net
} // namespace mk
#endif
