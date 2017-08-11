// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_POLLER_HPP
#define PRIVATE_LIBEVENT_POLLER_HPP

// # Libevent/poller
/*-
     _     _ _                          _                 _ _
    | |   (_) |__   _____   _____ _ __ | |_   _ __   ___ | | | ___ _ __
    | |   | | '_ \ / _ \ \ / / _ \ '_ \| __| | '_ \ / _ \| | |/ _ \ '__|
    | |___| | |_) |  __/\ V /  __/ | | | |_  | |_) | (_) | | |  __/ |
    |_____|_|_.__/ \___| \_/ \___|_| |_|\__| | .__/ \___/|_|_|\___|_|
                                             |_|
*/
/// \section Libevent/poller
/// \brief Header-only implementation of mk::Reactor based on libevent.

#include "private/common/mock.hpp"                 // for MK_MOCK
#include "private/common/worker.hpp"               // for mk::Worker
#include <cassert>                                 // for assert
#include <event2/event.h>                          // for event_base_*
#include <event2/thread.h>                         // for evthread_use_*
#include <event2/util.h>                           // for evutil_socket_t
#include <functional>                              // for std::function
#include <measurement_kit/common/callback.hpp>     // for mk::Callback
#include <measurement_kit/common/error.hpp>        // for mk::Error
#include <measurement_kit/common/logger.hpp>       // for mk::warn
#include <measurement_kit/common/non_copyable.hpp> // for mk::NonCopyable
#include <measurement_kit/common/non_movable.hpp>  // for mk::NonMovable
#include <measurement_kit/common/reactor.hpp>      // for mk::Reactor
#include <measurement_kit/common/socket.hpp>       // for mk::socket_t
#include <measurement_kit/common/utils.hpp>        // for mk::timeval_init
#include <measurement_kit/portable/netdb.h>        // for getaddrinfo
#include <measurement_kit/portable/sys/socket.h>   // for socket
#include <memory>                                  // for std::unique_ptr
#include <signal.h>                                // for sigaction
#include <stdexcept>                               // for std::runtime_error
#include <system_error>                            // for std::error_condition
#include <tuple>                                   // for std::tuple
#include <utility>                                 // for std::move

extern "C" {
static inline void mk_call_later_cb(evutil_socket_t, short, void *);
static inline void mk_pollfd_cb(evutil_socket_t, short, void *);
static inline void mk_periodic_cb(evutil_socket_t, short, void *);
}

namespace mk {
namespace libevent {

// ## EventUptr
/*-
     _____                 _   _   _       _
    | ____|_   _____ _ __ | |_| | | |_ __ | |_ _ __
    |  _| \ \ / / _ \ '_ \| __| | | | '_ \| __| '__|
    | |___ \ V /  __/ | | | |_| |_| | |_) | |_| |
    |_____| \_/ \___|_| |_|\__|\___/| .__/ \__|_|
                                    |_|
*/
/// \subsection EventUptr
/// \brief Convenience wrapper for managing events

struct EventDeleter {
    void operator()(event *p) {
        if (p) {
            event_free(p);
        }
    }
};
using EventUptr = std::unique_ptr<event, EventDeleter>;

// ## Poller
/*-
     ____       _ _
    |  _ \ ___ | | | ___ _ __
    | |_) / _ \| | |/ _ \ '__|
    |  __/ (_) | | |  __/ |
    |_|   \___/|_|_|\___|_|
*/
/// \subsection Poller
/// \brief Here we have our main class.

/// \brief mk::Reactor implementation using libevent.
template <MK_MOCK(event_base_new), MK_MOCK(event_base_once),
          MK_MOCK(event_base_dispatch), MK_MOCK(event_base_loopbreak),
          MK_MOCK(event_new), MK_MOCK(event_add)>
class Poller : public Reactor, public NonCopyable, public NonMovable {
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
    class LibeventLibrary {
      public:
        static LibeventLibrary *global() {
            static LibeventLibrary singleton;
            return &singleton;
        }

        LibeventLibrary() {
            if (evthread_use_pthreads() != 0) {
                throw std::runtime_error("evthread_use_pthreads");
            }
            struct sigaction sa {};
            sa.sa_handler = SIG_IGN;
            if (sigaction(SIGPIPE, &sa, nullptr) != 0) {
                throw std::runtime_error("sigaction");
            }
        }
    };

    LibeventLibrary<> *library = LibeventLibrary<>::global();

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

    Poller() {
        if ((evbase = event_base_new()) == nullptr) {
            throw std::runtime_error("event_base_new");
        }
    }

    ~Poller() override { event_base_free(evbase); }

    event_base *get_event_base() override { return evbase; }

    void run() override {
        // Register a persistent periodic event to make sure that the event
        // loop is not going to exit if we run out of events. This is required
        // because, when we're resolving DNS, we do that in a background
        // thread without typically having any event in the event loop which
        // will exit. To avoid this, here's the persistent event hack. Another
        // possible fix (more elegant), would be to move the worker into the
        // poller and consider the poller not done as long as we have pending
        // callbacks, pending I/O (this checked by libevent) and pending
        // worker threads to work on.
        //
        // Note that the libevent v2.1.x has a flag to obtain the behavior
        // described above, but the v2.0.x libevent doesn't.

        timeval ten_seconds;
        EventUptr persist{event_new(evbase, -1, EV_PERSIST | EV_TIMEOUT,
                                    mk_periodic_cb, nullptr)};
        if (!persist) {
            throw std::runtime_error("event_new");
        }
        if (event_add(persist.get(), timeval_init(&ten_seconds, 10.0)) != 0) {
            throw std::runtime_error("event_add");
        }

        auto rv = event_base_dispatch(evbase);
        if (rv < 0) {
            throw std::runtime_error("event_base_dispatch");
        }
        if (rv > 1) {
            mk::warn("loop: no pending and/or active events");
        }
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

    void pollfd(socket_t sockfd, short events, double timeout,
                Callback<Error, short> &&callback) override {
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
    (*cbp)(err, flags);
    delete cbp;
}

static inline void mk_periodic_cb(evutil_socket_t, short, void *) {
    /* NOTHING */
}
