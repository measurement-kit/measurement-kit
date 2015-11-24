// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>                              // for seconds
#include <functional>                          // for function
#include <measurement_kit/common/async.hpp>    // for Async
#include <measurement_kit/ooni/base_test.hpp>  // for BaseTest
#include <measurement_kit/common/var.hpp>      // for Var
#include <ratio>                               // for ratio
#include <thread>                              // for sleep_for

namespace measurement_kit {

namespace common {
class NetTest;
}

namespace ooni {
using namespace measurement_kit::common;

void BaseTest::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated poller, but the code is not yet ready for that.
    bool done = false;
    run([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

void BaseTest::run(std::function<void()> callback) {
    Async::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

} // namespace ooni
} // namespace measurement_kit
