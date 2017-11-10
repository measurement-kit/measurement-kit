// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/locked.hpp"
#include <measurement_kit/common.hpp>

TEST_CASE("The locked template works") {
    std::mutex mutex;
    double value = 0.0;

    SECTION("For non-void return values") {
        REQUIRE(value == 0.0); // See if I understand how Catch works
        REQUIRE(mk::locked(mutex, [&]() {
            value = 3.1415;
            return value;
        }) == value);
    }

    SECTION("For void return values") {
        REQUIRE(value == 0.0); // See if I understand how Catch works
        mk::locked(mutex, [&]() { value = 6.28; });
        REQUIRE(value == 6.28);
    }
}
