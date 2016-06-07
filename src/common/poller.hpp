// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_HPP
#define SRC_COMMON_POLLER_HPP

#include "src/common/utils.hpp"
#include <csignal>
#include <cstring>
#include <event2/event.h>
#include <event2/thread.h>
#include <functional>
#include <measurement_kit/common.hpp>
#include <stdexcept>

extern "C" {

void mk_call_soon_cb(evutil_socket_t, short, void *p);
void mk_do_periodic_cb(evutil_socket_t, short, void *ptr);

} // extern "C"
namespace mk {

template <MK_MOCK(evthread_use_pthreads), MK_MOCK(sigaction)>
class MkLibrarySingleton {
  private:
    MkLibrarySingleton() {
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

  public:
    static void ensure() {
        static MkLibrarySingleton<evthread_use_pthreads, sigaction> singleton;
    }
};

class Poller : public Reactor {
  public:

    // Poller cannot be a template because it must derive from Reactor but we
    // need templates in the unit test and unfortunately the templatized
    // constructor can't be called (http://stackoverflow.com/questions/3960849)
    // hence two constructors: the normal one that calls `init_()` and the
    // one receiving `nullptr` as argument which does not call `init_()` such
    // that we can call this template function in regress tests.
    template <MK_MOCK(evthread_use_pthreads), MK_MOCK(sigaction),
              MK_MOCK(event_base_new), MK_MOCK(event_base_free)>
    void init_() {
        MkLibrarySingleton<evthread_use_pthreads, sigaction>::ensure();
        base_ = Var<event_base>(event_base_new(), [](event_base *p) {
            if (p != nullptr) {
                event_base_free(p);
            }
        });
        if (!base_)
            throw std::bad_alloc();
    }
    Poller(std::nullptr_t) {}
    Poller() { init_(); }

    ~Poller() {}

    event_base *get_event_base() override { return base_.get(); }

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb) override;

    template <MK_MOCK(event_base_once)>
    void call_later_impl(double timeo, std::function<void()> cb) {
        timeval tv, *tvp = timeval_init(&tv, timeo);
        auto cbp = new std::function<void()>(cb);
        if (event_base_once(base_.get(), -1, EV_TIMEOUT, mk_call_soon_cb, cbp,
                            tvp) != 0) {
            delete cbp;
            throw std::runtime_error("event_base_once() failed");
        }
    }

    void call_later(double timeo, std::function<void()> cb) override;

    void loop_with_initial_event(std::function<void()> cb) override {
        call_soon(cb);
        loop();
    }

    template<MK_MOCK(event_new), MK_MOCK(event_free), MK_MOCK(event_add),
             MK_MOCK(event_base_dispatch)>
    void loop_impl() {
        // Register a persistent periodic event to make sure that the event
        // loop is not going to exit if we run out of events. This is required
        // to make sure that the ordinary libevent loop works like tor event
        // loop (also based on libevent), which does not exit in any case.
        //
        // Note that the development version of libevent has a flag to implement
        // the behavior described above, but the stable libevent doesn't.
        timeval ten_seconds;
        Var<event> persist(
            event_new(base_.get(), -1, EV_PERSIST, mk_do_periodic_cb, this),
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
        auto result = event_base_dispatch(base_.get());
        if (result < 0)
            throw std::runtime_error("event_base_dispatch() failed");
        if (result == 1)
            warn("loop: no pending and/or active events");
    }

    void loop() override;

    template <MK_MOCK(event_base_loop)> void loop_once_impl() {
        auto result = event_base_loop(base_.get(), EVLOOP_ONCE);
        if (result < 0)
            throw std::runtime_error("event_base_loop() failed");
        if (result == 1)
            warn("loop: no pending and/or active events");
    }

    void loop_once() override;

    template <MK_MOCK(event_base_loopbreak)> void break_loop_impl() {
        if (event_base_loopbreak(base_.get()) != 0)
            throw std::runtime_error("event_base_loopbreak() failed");
    }

    void break_loop() override;

    // BEGIN internal functions used to test periodic event functionality
    void handle_periodic_();
    void on_periodic_(std::function<void(Poller *)> callback);
    // END internal functions used to test periodic event functionality

  private:
    Var<event_base> base_;
    Delegate<Poller *> periodic_cb_;
};

} // namespace mk
#endif
