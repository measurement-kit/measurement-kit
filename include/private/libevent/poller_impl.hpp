// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_POLLER_IMPL_HPP
#define PRIVATE_LIBEVENT_POLLER_IMPL_HPP

#include "private/common/mock.hpp"

#include "private/common/mock.hpp"

#include "../common/utils.hpp"
#include "../libevent/poller.hpp"

#include <measurement_kit/common.hpp>

#include <csignal>
#include <cstring>
#include <event2/event.h>
#include <event2/thread.h>

extern "C" {

void mk_call_later_cb(evutil_socket_t, short, void *p);
void mk_loop_periodic_cb(evutil_socket_t, short, void *ptr);
void mk_pollfd_cb(evutil_socket_t, short, void *p);

} // extern "C"
namespace mk {
namespace libevent {

template <MK_MOCK(evthread_use_pthreads), MK_MOCK(sigaction)>
void library_singleton_() {
    static bool initialized = 0;
    if (initialized) {
        return;
    }
    initialized = true;
    if (evthread_use_pthreads() != 0) {
        throw std::runtime_error("evthread_use_pthreads() failed");
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, nullptr) != 0) {
        throw std::runtime_error("sigaction() failed");
    }
}

template <MK_MOCK(evthread_use_pthreads), MK_MOCK(sigaction),
          MK_MOCK(event_base_new), MK_MOCK(event_base_free)>
Var<event_base> poller_alloc_evbase() {
    // When we create the first event-base ever we also run the library
    // singleton, to initialize locking and signals.
    library_singleton_<evthread_use_pthreads, sigaction>();
    Var<event_base> base(event_base_new(), [](event_base *p) {
        if (p != nullptr) {
            event_base_free(p);
        }
    });
    if (!base) {
        throw std::bad_alloc();
    }
    return base;
}

template <MK_MOCK(event_base_once)>
void poller_call_later(Var<event_base> base, double timeo, Callback<> cb) {
    timeval tv;
    auto cbp = new Callback<>(cb);
    if (event_base_once(base.get(), -1, EV_TIMEOUT, mk_call_later_cb, cbp,
                        timeval_init(&tv, timeo)) != 0) {
        delete cbp;
        throw std::runtime_error("event_base_once() failed");
    }
}

template<MK_MOCK(event_new), MK_MOCK(event_free), MK_MOCK(event_add),
         MK_MOCK(event_base_dispatch)>
void poller_loop(Var<event_base> base, Poller *poller) {
    // Register a persistent periodic event to make sure that the event
    // loop is not going to exit if we run out of events. This is required
    // to make sure that the ordinary libevent loop works like tor event
    // loop (also based on libevent), which does not exit in any case.
    //
    // Note that the development version of libevent has a flag to implement
    // the behavior described above, but the stable libevent doesn't.

    timeval ten_seconds;
    Var<event> persist(
        event_new(base.get(), -1, EV_PERSIST, mk_loop_periodic_cb, poller),
        [](event *p) {
            if (p != nullptr) {
                event_free(p);
            }
        });
    if (!persist) {
        throw std::runtime_error("event_new() failed");
    }
    if (event_add(persist.get(), timeval_init(&ten_seconds, 10.0)) != 0) {
        throw std::runtime_error("event_add() failed");
    }

    auto result = event_base_dispatch(base.get());
    if (result < 0) {
        throw std::runtime_error("event_base_dispatch() failed");
    }
    if (result == 1) {
        warn("loop: no pending and/or active events");
    }
}

template <MK_MOCK(event_base_loop)>
void poller_loop_once(Var<event_base> base) {
    auto result = event_base_loop(base.get(), EVLOOP_ONCE);
    if (result < 0) {
        throw std::runtime_error("event_base_loop() failed");
    }
    if (result == 1) {
        warn("loop: no pending and/or active events");
    }
}

template <MK_MOCK(event_base_loopbreak)>
void poller_break_loop(Var<event_base> base) {
    if (event_base_loopbreak(base.get()) != 0) {
        throw std::runtime_error("event_base_loopbreak() failed");
    }
}

template <MK_MOCK(event_base_once)>
void poller_pollfd(
        Var<event_base> base,
        socket_t sockfd,
        short events,
        Callback<Error, short> callback,
        double timeo) {
    timeval tv;
    short evflags = EV_TIMEOUT;
    if ((events & MK_POLLIN) != 0) {
        evflags |= EV_READ;
    }
    if ((events & MK_POLLOUT) != 0) {
        evflags |= EV_WRITE;
    }
    auto cbp = new Callback<Error, short>(callback);
    if (event_base_once(base.get(), sockfd, evflags, mk_pollfd_cb, cbp,
                        timeval_init(&tv, timeo)) != 0) {
        delete cbp;
        throw std::runtime_error("event_base_once() failed");
    }
}

} // namespace libevent
} // namespace mk
#endif
