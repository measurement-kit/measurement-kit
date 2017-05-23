// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/poller.hpp"
#include "../common/remote_reactor.hpp"

namespace mk {

/*static*/ Var<Reactor> Reactor::make_remote() {
    return Var<Reactor>{new RemoteReactor};
}

/*static*/ Var<Reactor> Reactor::make() {
    return Var<Reactor>(new libevent::Poller);
}

Reactor::~Reactor() {}

/*static*/ Var<Reactor> Reactor::global() {
    // XXX protect singleton?
    static Var<Reactor> singleton = make();
    return singleton;
}

/*static*/ Var<Reactor> Reactor::global_remote() {
    // XXX protect singleton?
    static Var<Reactor> singleton = make_remote();
    return singleton;
}

void call_soon(Callback<> callback, Var<Reactor> reactor) {
    reactor->call_soon(callback);
}

void call_later(double delta, Callback<> callback, Var<Reactor> reactor) {
    reactor->call_later(delta, callback);
}

void loop_with_initial_event(Callback<> callback, Var<Reactor> reactor) {
    reactor->loop_with_initial_event(callback);
}

void loop(Var<Reactor> reactor) {
    reactor->loop();
}

void loop_once(Var<Reactor> reactor) {
    reactor->loop_once();
}

void break_loop(Var<Reactor> reactor) {
    reactor->break_loop();
}

} // namespace mk
