// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_POLLER_HPP
#define MEASUREMENT_KIT_COMMON_POLLER_HPP

#include <functional>
#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/funcs.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/var.hpp>

struct event_base;

namespace mk {

class Poller : public NonCopyable, public NonMovable {
  public:
    static Var<Poller> make() { return Var<Poller>(new Poller); }

    Poller(Libs *libs = nullptr);
    ~Poller();

    event_base *get_event_base() { return base_; }

    /// Call the function at the beginning of next I/O loop.
    /// \param cb The function to be called soon.
    /// \throw Error if the underlying libevent call fails.
    void call_soon(std::function<void()> cb);

    void call_later(double, std::function<void()> cb);

    void loop_with_initial_event(std::function<void()> cb) {
        call_soon(cb);
        loop();
    }

    void loop();

    void loop_once();

    void break_loop();

    static Var<Poller> global() {
        static Var<Poller> singleton(new Poller);
        return singleton;
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

/*
 * Syntactic sugar:
 */

inline void loop_with_initial_event(std::function<void()> cb) {
    Poller::global()->loop_with_initial_event(cb);
}

inline void loop() { Poller::global()->loop(); }

inline void loop_once() { Poller::global()->loop_once(); }

inline void break_loop() { Poller::global()->break_loop(); }

} // namespace mk
#endif
