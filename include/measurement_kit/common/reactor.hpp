// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <functional>
#include <measurement_kit/common/var.hpp>

struct event_base;

namespace mk {

class Reactor {
  public:
    static Var<Reactor> make();
    virtual ~Reactor() {}

    virtual void call_soon(std::function<void()> cb) = 0;
    virtual void call_later(double, std::function<void()> cb) = 0;

    virtual void loop_with_initial_event(std::function<void()> cb) = 0;
    virtual void loop() = 0;
    virtual void loop_once() = 0;
    virtual void break_loop() = 0;

    virtual event_base *get_event_base() = 0;

    static Var<Reactor> global() {
        static Var<Reactor> singleton = make();
        return singleton;
    }
};

inline void call_soon(std::function<void()> cb) {
    Reactor::global()->call_soon(cb);
}

inline void call_later(double t, std::function<void()> cb) {
    Reactor::global()->call_later(t, cb);
}

inline void loop_with_initial_event(std::function<void()> cb) {
    Reactor::global()->loop_with_initial_event(cb);
}

void loop_with_initial_event_and_connectivity(std::function<void()> cb);

inline void loop() { Reactor::global()->loop(); }

inline void loop_once() { Reactor::global()->loop_once(); }

inline void break_loop() { Reactor::global()->break_loop(); }

} // namespace mk
#endif
