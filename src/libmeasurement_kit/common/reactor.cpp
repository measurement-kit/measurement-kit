// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/detail/locked.hpp>
#include <measurement_kit/common/libevent/reactor.hpp>

namespace mk {

/*static*/ SharedPtr<Reactor> Reactor::make() {
    return locked_global([]() { return SharedPtr<Reactor>{new libevent::Reactor<>}; });
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
