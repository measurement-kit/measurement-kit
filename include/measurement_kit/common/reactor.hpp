// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/var.hpp>

// Deprecated in v0.4.x, will be removed in v0.5.x
struct event_base;

namespace mk {

class Reactor {
  public:
    static Var<Reactor> make();
    virtual ~Reactor();

    void call_soon(Callback<> cb);
    virtual void call_later(double, Callback<> cb) = 0;

    void loop_with_initial_event(Callback<> cb);
    virtual void loop() = 0;
    virtual void loop_once() = 0;
    virtual void break_loop() = 0;

    // Deprecated in v0.4.x, will be removed in v0.5.x
    virtual event_base *get_event_base() = 0;

    // Introduced as aliases in v0.4.x
    void run_with_initial_event(Callback<> cb) {
        loop_with_initial_event(cb);
    }
    void run() { loop(); }
    void stop() { break_loop(); }

    static Var<Reactor> global();
};

void call_soon(Callback<>, Var<Reactor> = Reactor::global());
void call_later(double, Callback<>, Var<Reactor> = Reactor::global());
void loop_with_initial_event(Callback<>, Var<Reactor> = Reactor::global());
void loop(Var<Reactor> = Reactor::global());
void loop_once(Var<Reactor> = Reactor::global());
void break_loop(Var<Reactor> = Reactor::global());

// Introduced as aliases in v0.4.x
inline void run_with_initial_event(Callback<> callback,
        Var<Reactor> reactor = Reactor::global()) {
    loop_with_initial_event(callback, reactor);
}
inline void run(Var<Reactor> reactor = Reactor::global()) {
    loop(reactor);
}
inline void stop(Var<Reactor> reactor = Reactor::global()) {
    break_loop(reactor);
}

} // namespace mk
#endif
