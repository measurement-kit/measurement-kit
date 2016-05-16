// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>
#include <functional>
#include <measurement_kit/common.hpp>
#include <ratio>
#include <thread>

namespace mk {

void NetTestDsl::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated reactor, but the code is not yet ready for that.
    bool done = false;
    run([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

void NetTestDsl::run(std::function<void()> callback) {
    Runner::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

} // namespace mk
