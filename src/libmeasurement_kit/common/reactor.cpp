// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/libevent_reactor.hpp"
#include "private/common/locked.hpp"

namespace mk {

/*static*/ SharedPtr<Reactor> Reactor::make() {
    return SharedPtr<Reactor>{std::make_shared<LibeventReactor<>>()};
}

Reactor::~Reactor() {}

void Reactor::run_with_initial_event(Callback<> &&cb) {
    call_soon(std::move(cb));
    run();
}

/*static*/ SharedPtr<Reactor> Reactor::global() {
    return locked_global([]() {
        static SharedPtr<Reactor> singleton = make();
        return singleton;
    });
}

} // namespace mk
