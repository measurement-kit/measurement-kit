// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/nettests.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace mk;

TEST_CASE("Synchronous NDT test") {
    nettests::MultiNdtTest{}
        .set_verbosity(MK_LOG_INFO)
        .run();
}

TEST_CASE("Asynchronous NDT test") {
    bool done = false;
    nettests::MultiNdtTest{}
        .set_verbosity(MK_LOG_INFO)
        .start([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

#else
int main(){}
#endif
