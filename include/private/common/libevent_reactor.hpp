// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_LIBEVENT_REACTOR_HPP
#define PRIVATE_COMMON_LIBEVENT_REACTOR_HPP

// # Libevent Reactor

#include "private/common/locked.hpp"               // for mk::locked_global
#include "private/common/mock.hpp"                 // for MK_MOCK
#include "private/common/utils.hpp"                // for mk::timeval_init
#include "private/common/worker.hpp"               // for mk::Worker
#include <cassert>                                 // for assert
#include <event2/event.h>                          // for event_base_*
#include <event2/thread.h>                         // for evthread_use_*
#include <event2/util.h>                           // for evutil_socket_t
#include <measurement_kit/common/callback.hpp>     // for mk::Callback
#include <measurement_kit/common/error.hpp>        // for mk::Error
#include <measurement_kit/common/logger.hpp>       // for mk::warn
#include <measurement_kit/common/raw_ptr.hpp>      // for mk::RawPtr
#include <measurement_kit/common/reactor.hpp>      // for mk::Reactor
#include <measurement_kit/common/socket.hpp>       // for mk::socket_t
#include <signal.h>                                // for sigaction
#include <stdexcept>                               // for std::runtime_error
#include <utility>                                 // for std::move

extern "C" {
static inline void mk_pollfd_cb(evutil_socket_t, short, void *);
}

namespace mk {

// Deleter for an event_base pointer.
class EventBaseDeleter {
  public:
    void operator()(event_base *evbase) {
        if (evbase != nullptr) {
            event_base_free(evbase);
        }
    }
};

// LibeventReactor is an mk::Reactor implementation using libevent.
//
// The current implementation as of 2017-11-01 does not need to be explicitly
// non-copyable and non-movable. But, given that in the future we will need
// probably to pass `this` to some libevent functions, and that anyway it is
// always used as mk::SharedPtr<mk::Reactor>, it seems more robust to keep it
// explicitly non-copyable and non-movable.
template <MK_MOCK(event_base_new), MK_MOCK(event_base_once),
        MK_MOCK(event_base_dispatch), MK_MOCK(event_base_loopbreak)>
class LibeventReactor : public Reactor, public NonCopyable, public NonMovable {
  public:
    // ## Initialization

    template <MK_MOCK(evthread_use_pthreads), MK_MOCK(sigaction)>
    static inline void libevent_init_once() {
        return locked_global([]() {
            static bool initialized = false;
            if (initialized) {
                return;
            }
            mk::debug("initializing libevent once");
            if (evthread_use_pthreads() != 0) {
                throw std::runtime_error("evthread_use_pthreads");
            }
            struct sigaction sa = {};
            sa.sa_handler = SIG_IGN;
            if (sigaction(SIGPIPE, &sa, nullptr) != 0) {
                throw std::runtime_error("sigaction");
            }
            initialized = true;
        });
    }

    LibeventReactor() {
        libevent_init_once();
        evbase.reset(event_base_new());
        if (evbase.get() == nullptr) {
            throw std::runtime_error("event_base_new");
        }
    }

    ~LibeventReactor() override {}

    // ## Event loop management

    event_base *get_event_base() override { return evbase; }

    void run() override {
        do {
            auto ev_status = event_base_dispatch(evbase);
            if (ev_status < 0) {
                throw std::runtime_error("event_base_dispatch");
            }
            /*
                Explanation: event_base_loop() returns one when there are no
                pending events. In such case, before leaving the event loop, we
                make sure we have no pending background threads. They are, as
                of now, mostly used to perform DNS queries with getaddrinfo(),
                which is blocking. If there are threads running, treat them
                like pending events, even though they are not managed by
                libevent, and continue running the loop. To avoid spawning
                and to be sure we're ready to deal /pronto/ with any upcoming
                libevent event, schedule a call for the near future so to
                keep the libevent loop active, and ready to react.

                The exact possible values for `ev_status` are -1, 0, and +1, but
                I have coded more broad checks for robustness.
            */
            if (ev_status > 0 && worker.concurrency() <= 0) {
                mk::warn("reactor: no pending and/or active events");
                break;
            }
            call_later(0.250, []() {});
        } while (true);
    }

    void stop() override {
        if (event_base_loopbreak(evbase) != 0) {
            throw std::runtime_error("event_base_loopbreak");
        }
    }

    // ## Call later

    void call_in_thread(SharedPtr<Logger> logger, Callback<> &&cb) override {
        worker.call_in_thread(logger, std::move(cb));
    }

    void call_soon(Callback<> &&cb) override { call_later(0.0, std::move(cb)); }

    void call_later(double delay, Callback<> &&cb) override {
        // Note: according to libevent documentation, it is not necessary to
        // pass `EV_TIMEOUT` to get a timeout. But I find passing it more clear.
        pollfd(-1, EV_TIMEOUT, delay, [cb = std::move(cb)](Error, short) {
            cb();
        });
    }

    // ## Poll sockets

    void pollin_once(socket_t fd, double timeo, Callback<Error> &&cb) override {
        pollfd(fd, EV_READ, timeo, [cb = std::move(cb)](Error err, short) {
            cb(std::move(err));
        });
    }

    void pollout_once(
            socket_t fd, double timeo, Callback<Error> &&cb) override {
        pollfd(fd, EV_WRITE, timeo, [cb = std::move(cb)](Error err, short) {
            cb(std::move(err));
        });
    }

    // ## Internals

    void pollfd(socket_t sockfd, short evflags, double timeout,
            Callback<Error, short> &&callback) {
        timeval tv{};
        auto cbp = new Callback<Error, short>(callback);
        if (event_base_once(evbase, sockfd, evflags, mk_pollfd_cb, cbp,
                    timeval_init(&tv, timeout)) != 0) {
            delete cbp;
            throw std::runtime_error("event_base_once");
        }
    }

    static void pollfd_cb(short evflags, void *opaque) {
        auto cbp = static_cast<mk::Callback<mk::Error, short> *>(opaque);
        mk::Error err = mk::NoError();
        assert((evflags & (~(EV_TIMEOUT | EV_READ | EV_WRITE))) == 0);
        if ((evflags & EV_TIMEOUT) != 0) {
            err = mk::TimeoutError();
        }
        // In case of exception here, the stack is going to unwind, tearing down
        // the libevent loop and leaking forever `cbp` and the event once that
        // was used to invoke this callback.
        (*cbp)(std::move(err), evflags);
        delete cbp;
    }

  private:
    // ## Private attributes

    RawPtr<event_base, EventBaseDeleter> evbase;
    Worker worker;
};

} // namespace mk

// ## C linkage callbacks

static inline void mk_pollfd_cb(evutil_socket_t, short evflags, void *opaque) {
    mk::LibeventReactor<>::pollfd_cb(evflags, opaque);
}
#endif
