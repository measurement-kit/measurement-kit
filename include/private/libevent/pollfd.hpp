// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_POLLFD_HPP
#define PRIVATE_LIBEVENT_POLLFD_HPP

#include "private/common/mock.hpp"             // for MK_MOCK
#include <algorithm>                           // for std::swap
#include <cassert>                             // for ::assert
#include <event2/event.h>                      // for event_base_once
#include <event2/util.h>                       // for evutil_socket_t
#include <measurement_kit/common/callback.hpp> // for mk::Callback
#include <measurement_kit/common/error.hpp>    // for mk::Error
#include <measurement_kit/common/reactor.hpp>  // for mk::Reactor
#include <measurement_kit/common/utils.hpp>    // for mk::timeval_init
#include <measurement_kit/common/var.hpp>      // for mk::Var
#include <measurement_kit/net/socket.hpp>      // for mk::net::os_socket_t
#include <stdexcept>                           // for std::runtime_error

extern "C" {
static inline void mk_pollfd_cb(evutil_socket_t, short, void *);
}

namespace mk {
namespace libevent {

template <MK_MOCK(event_base_once)>
void pollfd(net::os_socket_t sockfd, short evflags, double timeout,
            Reactor reactor, Callback<Error, short> &&callback) {
    timeval tv{};
    auto cbp = new Callback<Error, short>(callback);
    if (event_base_once(reactor.get_event_base(), sockfd, evflags,
                        mk_pollfd_cb, cbp, timeval_init(&tv, timeout)) != 0) {
        delete cbp;
        throw std::runtime_error("event_base_once");
    }
}

} // namespace libevent
} // namespace mk

static inline void mk_pollfd_cb(evutil_socket_t, short evflags, void *opaque) {
    using namespace mk;
    auto cbp = static_cast<Callback<Error, short> *>(opaque);
    assert((evflags & (~(EV_TIMEOUT | EV_READ | EV_WRITE))) == 0);
    Error err = NoError();
    if ((evflags & EV_TIMEOUT) != 0) {
        err = TimeoutError();
    }
    Callback<Error, short> cb;
    std::swap(cb, *cbp);
    delete cbp;
    cb(err, evflags);
}
#endif
