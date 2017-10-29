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
    mk::Worker worker;
    for (auto _: mk::range<size_t>(32)) {
        worker.call_in_thread([]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        });
    }
    for (;;) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        auto concurrency = worker.concurrency();
        std::cout << "Concurrency: " << concurrency << "\n";
        REQUIRE(concurrency <= worker.parallelism());
        if (concurrency == 0) {
            break;
        }
    }
}

TEST_CASE("The worker deals in a clean way with an exception") {
    mk::Worker worker;
    worker.set_parallelism(1);
    std::atomic<std::size_t> num_called{0};
    std::atomic<bool> seen_exc{false};
    worker.on_error([&](std::exception_ptr) {
        seen_exc = true;
    });
    for (auto _: mk::range<size_t>(4)) {
        worker.call_in_thread([&]() {
            using namespace std::chrono_literals;
            ++num_called;
            std::this_thread::sleep_for(1s);
        });
    }
    worker.call_in_thread([&]() {
        ++num_called;
        throw mk::MockedError();
    });
    for (auto _: mk::range<size_t>(4)) {
        worker.call_in_thread([&]() {
            ++num_called;
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        });
    }
    for (;;) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        auto concurrency = worker.concurrency();
        std::cout << "Concurrency: " << concurrency << "\n";
        REQUIRE(concurrency <= worker.parallelism());
        if (concurrency == 0) {
            break;
        }
    }
    REQUIRE(num_called == 5);
}
