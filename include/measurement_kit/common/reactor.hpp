// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_REACTOR_HPP
#define MEASUREMENT_KIT_COMMON_REACTOR_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/var.hpp>

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

    virtual event_base *get_event_base() = 0;

    static Var<Reactor> global();
};

void call_soon(Callback<>, Var<Reactor> = Reactor::global());
void call_later(double, Callback<>, Var<Reactor> = Reactor::global());
void loop_with_initial_event(Callback<>, Var<Reactor> = Reactor::global());
void loop(Var<Reactor> = Reactor::global());
void loop_once(Var<Reactor> = Reactor::global());
void break_loop(Var<Reactor> = Reactor::global());

void loop_with_initial_event_and_connectivity(Callback<> cb);

} // namespace mk
#endif
