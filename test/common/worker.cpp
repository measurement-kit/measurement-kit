// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/worker.hpp"
#include "private/common/range.hpp"

#include <measurement_kit/common.hpp>

#include <chrono>
#include <iostream>
#include <thread>

TEST_CASE("The worker is robust to submitting many tasks in a row") {
    auto worker = mk::SharedPtr<mk::Worker>::make();
    for (auto _: mk::range<int>(128)) {
        worker->call_in_thread([]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
        });
    }
    for (;;) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        auto concurrency = worker->concurrency();
        std::cout << "Concurrency: " << concurrency << "\n";
        REQUIRE(concurrency <= worker->parallelism());
        if (concurrency == 0) {
            break;
        }
    }
}
