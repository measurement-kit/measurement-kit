// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_LIBEVENT_REACTOR_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_LIBEVENT_REACTOR_HPP

// # Libevent Reactor

#include "src/libmeasurement_kit/common/locked.hpp"               // for mk::locked_global
#include "src/libmeasurement_kit/common/mock.hpp"                 // for MK_MOCK
#include "src/libmeasurement_kit/common/non_copyable.hpp"         // for mk::NonCopyable
#include "src/libmeasurement_kit/common/non_movable.hpp"          // for mk::NonMovable
#include "src/libmeasurement_kit/common/reactor.hpp"              // for mk::Reactor
#include "src/libmeasurement_kit/common/socket.hpp"               // for mk::socket_t
#include "src/libmeasurement_kit/common/utils.hpp"                // for mk::timeval_init
#include "src/libmeasurement_kit/common/unique_ptr.hpp"           // for mk::UniquePtr
#include "src/libmeasurement_kit/common/worker.hpp"               // for mk::Worker
#include <atomic>                                  // for std::atomic_bool
#include <cassert>                                 // for assert
#include <event2/event.h>                          // for event_base_*
#include <event2/thread.h>                         // for evthread_use_*
#include <event2/util.h>                           // for evutil_socket_t
#include "src/libmeasurement_kit/common/callback.hpp"     // for mk::Callback
#include <measurement_kit/common/data_usage.hpp>   // for mk::DataUsage
#include "src/libmeasurement_kit/common/error.hpp"        // for mk::Error
#include <measurement_kit/common/logger.hpp>       // for mk::warn
#include <mutex>                                   // for std::recursive_mutex
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

    template <
        // Depending on the platform we have different thread init funcs
#ifdef _WIN32
        MK_MOCK_AS(evthread_use_windows_threads, evthread_init)
#else
        MK_MOCK_AS(evthread_use_pthreads, evthread_init), MK_MOCK(sigaction)
#endif
        >
    static inline void libevent_init_once() {
        return locked_global([]() {
            static bool initialized = false;
            if (initialized) {
                return;
            }
            if (evthread_init() != 0) {
                throw std::runtime_error("evthread_init");
            }
            // SIGPIPE is not an issue on Windows
#ifndef _WIN32
            struct sigaction sa = {};
            sa.sa_handler = SIG_IGN;
            if (sigaction(SIGPIPE, &sa, nullptr) != 0) {
                throw std::runtime_error("sigaction");
            }
#endif
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

    event_base *get_event_base() override { return evbase.get(); }

    void run() override {
        do {
            auto ev_status = event_base_dispatch(evbase.get());
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
                libevent, and continue running the loop. To avoid spinning
                and to be sure we're ready to deal /pronto/ with any upcoming
                libevent event, schedule a call for the near future so to
                keep the libevent loop active, and ready to react.

                The exact possible values for `ev_status` are -1, 0, and +1, but
                I have coded more broad checks for robustness.
            */
            if (ev_status > 0 && worker.concurrency() <= 0) {
                break;
            }
            call_later(0.250, []() {});
        } while (true);
    }

    void stop() override {
        if (event_base_loopbreak(evbase.get()) != 0) {
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
        if (event_base_once(evbase.get(), sockfd, evflags, mk_pollfd_cb, cbp,
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

    // ## Data usage

    void with_current_data_usage(Callback<DataUsage &> &&cb) override {
        std::unique_lock<std::recursive_mutex> _{data_usage_mutex};
        cb(data_usage);
    }

  private:
    // ## Private attributes

    UniquePtr<event_base, EventBaseDeleter> evbase;
    std::recursive_mutex data_usage_mutex;
    DataUsage data_usage;
    Worker worker;
};

} // namespace mk

// ## C linkage callbacks

static inline void mk_pollfd_cb(evutil_socket_t, short evflags, void *opaque) {
    mk::LibeventReactor<>::pollfd_cb(evflags, opaque);
}
#endif
