// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "check_connectivity.hpp"
#include "poller.hpp"
#include <measurement_kit/common.hpp>

namespace mk {

/*static*/ Var<Reactor> Reactor::make() { return Var<Reactor>(new Poller); }

void loop_with_initial_event_and_connectivity(std::function<void()> cb) {
    if (!CheckConnectivity::is_down()) {
        loop_with_initial_event(cb);
    } else {
        warn("Test skipped because network is down");
    }
}

void reactor_call_soon(Var<Reactor> reactor, Callback<> callback) {
    reactor->call_soon(callback);
}

} // namespace mk
