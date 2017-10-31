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
#include <measurement_kit/common/non_copyable.hpp> // for mk::NonCopyable
#include <measurement_kit/common/non_movable.hpp>  // for mk::NonMovable
#include <measurement_kit/common/reactor.hpp>      // for mk::Reactor
#include <measurement_kit/common/socket.hpp>       // for mk::socket_t
#include <set>                                     // for std::set
#include <signal.h>                                // for sigaction
#include <stdexcept>                               // for std::runtime_error
#include <utility>                                 // for std::move

extern "C" {
static inline void mk_pollfd_cb(evutil_socket_t, short, void *);
}

namespace mk {

// mk::Reactor implementation using libevent.
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
        if ((evbase = event_base_new()) == nullptr) {
            throw std::runtime_error("event_base_new");
        }
    }

    ~LibeventReactor() override {
        pollfd_cleanup();
        event_base_free(evbase);
    }

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

    // ## Pending callbacks

    // Most reactor operations lead to the creation of a pending callback to be
    // called when the corresponding condition is met. The pending callback must
    // be managed through a shared pointer, whose ownership will be shared by
    // libevent code and the reactor itself, as detailed below.
    //
    // The pending callback structure contains a `cb` field that is the real
    // callback to be called. This callback takes two arguments. The error that
    // occurred, if any, and the flags indicating readability (EV_READ) and
    // writability (EV_WRITE).
    //
    // The `libevent_ref` shared pointer is a copy of the shared pointer
    // initially used to create the structure. The purpose of this field is to
    // keep the structure alive until libevent has finished using it.
    //
    // Another copy of the shared pointer is saved in the `pending` field of
    // the reactor. To remove this copy from `pending` when the callback is
    // called by libevent, we need to navigate from the callback itself to the
    // reactor. Hence the `reactor` pointer field.
    //
    // The `reactor` field will be initialized by pollfd() with `this`. It is
    // a safe assignment, since the pending callback lifecycle cannot be longer
    // than the lifecycle of the reactor. Note that we do not need to enforce
    // non-copyability and/or non-movability since the pending callback does
    // not manage the lifecycle of `reactor` and all other fields use RAII.
    //
    // The reason why we want the reactor to know about pending callbacks is to
    // avoid memory leaks when we exit abruply from the I/O loop, either because
    // stop() was called or because of an exception.
    class PendingCallback {
      public:
        Callback<Error, short> cb;
        SharedPtr<PendingCallback> libevent_ref;
        LibeventReactor *reactor = nullptr;
    };

    // The pollfd() method creates a pending callback from a socket (use -1 to
    // indicate no socket), I/O flags (optional; EV_READ and/or EV_WRITE); a
    // timeout (use a negative value to indicate no timeout), and a callback.
    void pollfd(socket_t sockfd, short evflags, double timeout,
            Callback<Error, short> &&callback) {
        timeval tv{};
        SharedPtr<PendingCallback> cbp{new PendingCallback};
        cbp->cb = std::move(callback);
        cbp->reactor = this;
        if (event_base_once(evbase, sockfd, evflags, mk_pollfd_cb, cbp.get(),
                    timeval_init(&tv, timeout)) != 0) {
            throw std::runtime_error("event_base_once");
        }
        // Pollfd() will create the pending callback through a shared pointer,
        // create the self reference indicating that libevent is using this
        // pending callback, and store another reference into `pending` to mean
        // that also the reactor itself is using the pending callback.
        cbp->libevent_ref = cbp;
        {
            // When pollfd() inserts the pending callback shared pointer into
            // the `pending` set, it protects it using a mutex. In theory, this
            // is not needed since libevent should be single threaded in this
            // case, but it's here for future robustness.
            std::unique_lock<std::recursive_mutex> _{pending_mutex};
            pending.insert(cbp);
        }
    }

    // When the destructor of the reactor is called, pollfd_cleanup() will
    // ensure that all pending-but-not-called callbacks (i.e. the ones that are
    // still inside `pending`) will be destroyed. To this end, it will remove
    // the libevent-is-using-it self reference. This is not a lie, as at this
    // point we're about to destroy the `event_base`.
    void pollfd_cleanup() {
        for (auto &cbp : pending) {
            cbp->libevent_ref = nullptr;
        }
    }

    // Libevent will eventually call pollfd_cb(), either because the specified
    // event has occurred, or because the timeout has expired.
    static void pollfd_cb(short evflags, void *opaque) {
        // Pollfd_cb() will copy the pending callback shared pointer on its
        // stack to enforce RAII semantics. Then it will remove all the other
        // shared pointers, since now the callback is not pending anymore.
        auto cbp = static_cast<PendingCallback *>(opaque)->libevent_ref;
        {
            // When removing from the `pending` field we protect it with a
            // mutex, for future robustness, as explained above.
            std::unique_lock<std::recursive_mutex> _{
                    cbp->reactor->pending_mutex};
            cbp->reactor->pending.erase(cbp);
        }
        cbp->libevent_ref = nullptr;

        mk::Error err = mk::NoError();
        assert((evflags & (~(EV_TIMEOUT | EV_READ | EV_WRITE))) == 0);
        if ((evflags & EV_TIMEOUT) != 0) {
            err = mk::TimeoutError();
        }

        // Finally, pollfd_cb() calls the function stored inside the pending
        // callback structure. Note that, if this function throws an exception,
        // we correctly free the pending callback because of RAII. Yet, we're
        // still leaking the event once used to execute this function.
        //
        // TODO: rewrite code to use a normal event to remove this limitation.
        cbp->cb(std::move(err), evflags);
    }

  private:
    // ## Private attributes

    event_base *evbase = nullptr;
    std::recursive_mutex pending_mutex;
    std::set<SharedPtr<PendingCallback>> pending;
    Worker worker;
};

} // namespace mk

// ## C linkage callbacks

static inline void mk_pollfd_cb(evutil_socket_t, short evflags, void *opaque) {
    mk::LibeventReactor<>::pollfd_cb(evflags, opaque);
}
#endif
