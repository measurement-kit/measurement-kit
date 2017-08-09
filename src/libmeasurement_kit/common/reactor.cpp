// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/libevent/poller.hpp"
#include <measurement_kit/common/locked.hpp>

namespace mk {

/*static*/ Var<Reactor> Reactor::make() {
    return locked_global([]() { return Var<Reactor>{new libevent::Poller<>}; });
}

Reactor::~Reactor() {}

void Reactor::run_with_initial_event(Callback<> &&cb) {
    call_soon(std::move(cb));
    loop();
}

/*static*/ Var<Reactor> Reactor::global() {
    return locked_global([]() {
        static Var<Reactor> singleton = make();
        return singleton;
    });
}

void call_soon(Callback<> &&callback, Var<Reactor> reactor) {
    reactor->call_soon(std::move(callback));
}

void call_later(double delta, Callback<> &&callback, Var<Reactor> reactor) {
    reactor->call_later(delta, std::move(callback));
}

void loop_with_initial_event(Callback<> &&callback, Var<Reactor> reactor) {
    reactor->run_with_initial_event(std::move(callback));
}

void loop(Var<Reactor> reactor) {
    reactor->run();
}

void break_loop(Var<Reactor> reactor) {
    reactor->stop();
}

} // namespace mk
