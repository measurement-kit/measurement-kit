// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>
#include <functional>
#include <measurement_kit/common.hpp>
#include <ratio>
#include <thread>
#include <future>

namespace mk {

void NetTest::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated reactor, but the code is not yet ready for that.
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    run([&promise]() { promise.set_value(true);});
    future.wait();
}

void NetTest::run(std::function<void()> callback) {
    Runner::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

NetTest::~NetTest() {}

} // namespace mk
