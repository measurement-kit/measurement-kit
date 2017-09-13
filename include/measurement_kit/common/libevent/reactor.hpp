// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LIBEVENT_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_LIBEVENT_REACTOR_HPP

// # Libevent/reactor
/*-
     _     _ _                          _                         _
    | |   (_) |__   _____   _____ _ __ | |_   _ __ ___  __ _  ___| |_ ___  _ __
    | |   | | '_ \ / _ \ \ / / _ \ '_ \| __| | '__/ _ \/ _` |/ __| __/ _ \| '__|
    | |___| | |_) |  __/\ V /  __/ | | | |_  | | |  __/ (_| | (__| || (_) | |
    |_____|_|_.__/ \___| \_/ \___|_| |_|\__| |_|  \___|\__,_|\___|\__\___/|_|
*/
/// \section Libevent/reactor
/// \brief Header-only implementation of mk::Reactor based on libevent.

#include <cassert>                                  // for assert
#include <event2/event.h>                           // for event_base_*
#include <event2/thread.h>                          // for evthread_use_*
#include <event2/util.h>                            // for evutil_socket_t
#include <functional>                               // for std::function
#include <measurement_kit/common/callback.hpp>      // for mk::Callback
#include <measurement_kit/common/detail/locked.hpp> // for mk::locked_global
#include <measurement_kit/common/detail/mock.hpp>   // for MK_MOCK
#include <measurement_kit/common/detail/utils.hpp>  // for mk::timeval_init
#include <measurement_kit/common/detail/worker.hpp> // for mk::Worker
#include <measurement_kit/common/error.hpp>         // for mk::Error
#include <measurement_kit/common/logger.hpp>        // for mk::warn
#include <measurement_kit/common/non_copyable.hpp>  // for mk::NonCopyable
#include <measurement_kit/common/non_movable.hpp>   // for mk::NonMovable
#include <measurement_kit/common/reactor.hpp>       // for mk::Reactor
#include <measurement_kit/common/socket.hpp>        // for mk::socket_t
#include <memory>                                   // for std::unique_ptr
#include <signal.h>                                 // for sigaction
#include <stdexcept>                                // for std::runtime_error
#include <system_error>                             // for std::error_condition
#include <tuple>                                    // for std::tuple
#include <utility>                                  // for std::move

extern "C" {
static inline void mk_call_later_cb(evutil_socket_t, short, void *);
static inline void mk_pollfd_cb(evutil_socket_t, short, void *);
}

namespace mk {
namespace libevent {

// ## Reactor
/*-
     ____       _ _
    |  _ \ ___ | | | ___ _ __
    | |_) / _ \| | |/ _ \ '__|
    |  __/ (_) | | |  __/ |
    |_|   \___/|_|_|\___|_|
*/
/// \subsection Reactor
/// \brief Here we have our main class.

/// \brief mk::Reactor implementation using libevent.
template <MK_MOCK(event_base_new), MK_MOCK(event_base_once),
          MK_MOCK(event_base_dispatch), MK_MOCK(event_base_loopbreak),
          MK_MOCK(event_new), MK_MOCK(event_add)>
class Reactor : public mk::Reactor, public NonCopyable, public NonMovable {
  public:
    // ### Library
    /*-
         _    _ _
        | |  (_) |__ _ _ __ _ _ _ _  _
        | |__| | '_ \ '_/ _` | '_| || |
        |____|_|_.__/_| \__,_|_|  \_, |
                                  |__/
    */
    /// \subsubsection Library
    /// \brief Code to make sure libevent is correctly configured.

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
            struct sigaction sa{};
            sa.sa_handler = SIG_IGN;
            if (sigaction(SIGPIPE, &sa, nullptr) != 0) {
                throw std::runtime_error("sigaction");
            }
            initialized = true;
        });
    }

    // ### Event loop
    /*-
         ___             _     _
        | __|_ _____ _ _| |_  | |___  ___ _ __
        | _|\ V / -_) ' \  _| | / _ \/ _ \ '_ \
        |___|\_/\___|_||_\__| |_\___/\___/ .__/
                                         |_|
    */
    /// \subsubsection Event loop
    /// \brief Here we setup and tear down libevent's event loop.

    event_base *evbase = nullptr;

    Reactor() {
        libevent_init_once();
        if ((evbase = event_base_new()) == nullptr) {
            throw std::runtime_error("event_base_new");
        }
    }

    ~Reactor() override { event_base_free(evbase); }

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

    // ### Call later
    /*-
          ___      _ _   _      _
         / __|__ _| | | | |__ _| |_ ___ _ _
        | (__/ _` | | | | / _` |  _/ -_) '_|
         \___\__,_|_|_| |_\__,_|\__\___|_|
    */
    /// \subsubsection Call later
    /// \brief Here we have code schedule callbacks at a later time.

    Worker worker;

    void run_in_background_thread(Callback<> &&cb) override {
        worker.run_in_background_thread(std::move(cb));
    }

    void call_soon(Callback<> &&cb) override { call_later(0.0, std::move(cb)); }

    /*
        Note: according to libevent documentation, it is not necessary to
        pass `EV_TIMEOUT` to get a timeout. But I find passing it more clear.
    */

    void call_later(double delay, Callback<> &&cb) override {
        auto tv = timeval{};
        auto cbp = new Callback<>{std::move(cb)};
        if (event_base_once(evbase, -1, EV_TIMEOUT, mk_call_later_cb, cbp,
                            timeval_init(&tv, delay)) != 0) {
            delete cbp;
            throw std::runtime_error("event_base_once");
        }
    }

    // ### Pollfd
    /*-
         ____       _ _  __     _
        |  _ \ ___ | | |/ _| __| |
        | |_) / _ \| | | |_ / _` |
        |  __/ (_) | | |  _| (_| |
        |_|   \___/|_|_|_|  \__,_|
    */
    /// \subsection Pollfd
    /// \brief Code to poll system file descriptors.

#define MK_POLLIN (1 << 0)

#define MK_POLLOUT (1 << 1)

    void pollfd(socket_t sockfd, short events, double timeout,
                Callback<Error, short> &&callback) {
        timeval tv{};
        short evflags = EV_TIMEOUT;
        if ((events & MK_POLLIN) != 0) {
            evflags |= EV_READ;
        }
        if ((events & MK_POLLOUT) != 0) {
            evflags |= EV_WRITE;
        }
        auto cbp = new Callback<Error, short>(callback);
        if (event_base_once(evbase, sockfd, evflags, mk_pollfd_cb, cbp,
                            timeval_init(&tv, timeout)) != 0) {
            delete cbp;
            throw std::runtime_error("event_base_once");
        }
    }

    void pollin(socket_t fd, double timeo, Callback<Error> &&cb) override {
        pollfd(fd, MK_POLLIN, timeo, [cb = std::move(cb)](Error err, short) {
            cb(std::move(err));
        });
    }

    void pollout(socket_t fd, double timeo, Callback<Error> &&cb) override {
        pollfd(fd, MK_POLLOUT, timeo, [cb = std::move(cb)](Error err, short) {
            cb(std::move(err));
        });
    }
};

} // namespace libevent
} // namespace mk
#endif

// ## C linkage callbacks
/*-
      ___   _ _      _                           _ _ _             _
     / __| | (_)_ _ | |____ _ __ _ ___   __ __ _| | | |__  __ _ __| |__ ___
    | (__  | | | ' \| / / _` / _` / -_) / _/ _` | | | '_ \/ _` / _| / /(_-<
     \___| |_|_|_||_|_\_\__,_\__, \___| \__\__,_|_|_|_.__/\__,_\__|_\_\/__/
                             |___/

*/
/// \subsection C linkage callbacks
/// \brief Here we have all the callbacks called directly by C code.

static inline void mk_call_later_cb(evutil_socket_t, short evflags,
                                    void *opaque) {
    assert((evflags & (~(EV_TIMEOUT))) == 0);
    auto cbp = static_cast<mk::Callback<> *>(opaque);
    // In case of exception here, the stack is going to unwind, tearing down
    // the libevent loop and leaking forever `cbp` and the event once that was
    // used to invoke this callback.
    (*cbp)();
    delete cbp;
}

static inline void mk_pollfd_cb(evutil_socket_t, short evflags, void *opaque) {
    auto cbp = static_cast<mk::Callback<mk::Error, short> *>(opaque);
    mk::Error err = mk::NoError();
    short flags = 0;
    assert((evflags & (~(EV_TIMEOUT | EV_READ | EV_WRITE))) == 0);
    if ((evflags & EV_TIMEOUT) != 0) {
        err = mk::TimeoutError();
    }
    if ((evflags & EV_READ) != 0) {
        flags |= MK_POLLIN;
    }
    if ((evflags & EV_WRITE) != 0) {
        flags |= MK_POLLOUT;
    }
    // In case of exception here, the stack is going to unwind, tearing down
    // the libevent loop and leaking forever `cbp` and the event once that was
    // used to invoke this callback.
    (*cbp)(std::move(err), flags);
    delete cbp;
}
