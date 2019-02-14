// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/libevent_reactor.hpp"
#include "src/libmeasurement_kit/common/locked.hpp"

namespace mk {

/*static*/ SharedPtr<Reactor> Reactor::make() {
    return SharedPtr<Reactor>{std::make_shared<LibeventReactor<>>()};
}

Reactor::~Reactor() {}

void Reactor::run_with_initial_event(Callback<> &&cb) {
    call_soon(std::move(cb));
    run();
}

} // namespace mk
