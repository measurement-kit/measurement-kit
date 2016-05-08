// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_COMMON_POLLER_HPP
#define SRC_COMMON_POLLER_HPP

#include "src/common/libs_impl.hpp"
#include "src/common/utils.hpp"
#include <event2/thread.h>
#include <functional>
#include <measurement_kit/common.hpp>
#include <stdexcept>

extern "C" {

void mk_call_soon_cb(evutil_socket_t, short, void *p);
void mk_do_periodic_cb(evutil_socket_t, short, void *ptr);

} // extern "C"
namespace mk {

class EvThreadSingleton {
  private:
    EvThreadSingleton() {
        if (evthread_use_pthreads() != 0) {
            throw std::runtime_error("evthread_use_pthreads() failed");
        }
    }

  public:
    static void ensure() {
        static EvThreadSingleton singleton;
    }
};

class Poller : public NonCopyable, public NonMovable, public Reactor {
  public:
    Poller(Libs *libs = nullptr) {
        if (libs != nullptr)
            libs_ = libs;
        EvThreadSingleton::ensure();
        if ((base_ = libs_->event_base_new()) == nullptr)
            throw std::bad_alloc();
    }

    ~Poller() { libs_->event_base_free(base_); }

    event_base *get_event_base() override { return base_; }

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb) override;

    void call_later(double timeo, std::function<void()> cb) override {
        timeval tv, *tvp = timeval_init(&tv, timeo);
        auto cbp = new std::function<void()>(cb);
        if (event_base_once(base_, -1, EV_TIMEOUT, mk_call_soon_cb, cbp, tvp) !=
            0) {
            delete cbp;
            throw std::runtime_error("event_base_once() failed");
        }
    }

    void loop_with_initial_event(std::function<void()> cb) override {
        call_soon(cb);
        loop();
    }

    void loop() override {
        // Register a persistent periodic event to make sure that the event
        // loop is not going to exit if we run out of events. This is required
        // to make sure that the ordinary libevent loop works like tor event
        // loop (also based on libevent), which does not exit in any case.
        //
        // Note that the development version of libevent has a flag to implement
        // the behavior described above, but the stable libevent doesn't.
        timeval ten_seconds;
        Var<event> persist(
            ::event_new(base_, -1, EV_PERSIST, mk_do_periodic_cb, this),
            [](event *p) {
                if (p != nullptr) {
                    ::event_free(p);
                }
            });
        if (!persist) {
            throw std::runtime_error("event_new() failed");
        }
        if (event_add(persist.get(), timeval_init(&ten_seconds, 10.0)) != 0) {
            throw std::runtime_error("event_add() failed");
        }
        auto result = libs_->event_base_dispatch(base_);
        if (result < 0)
            throw std::runtime_error("event_base_dispatch() failed");
        if (result == 1)
            warn("loop: no pending and/or active events");
    }

    void loop_once() override {
        auto result = libs_->event_base_loop(base_, EVLOOP_ONCE);
        if (result < 0)
            throw std::runtime_error("event_base_loop() failed");
        if (result == 1)
            warn("loop: no pending and/or active events");
    }

    void break_loop() override {
        if (libs_->event_base_loopbreak(base_) != 0)
            throw std::runtime_error("event_base_loopbreak() failed");
    }

    // BEGIN internal functions used to test periodic event functionality
    void handle_periodic_();
    void on_periodic_(std::function<void(Poller *)> callback);
    // END internal functions used to test periodic event functionality

  private:
    event_base *base_;
    Libs *libs_ = get_global_libs();
    SafelyOverridableFunc<void(Poller *)> periodic_cb_;
};

} // namespace mk
#endif
