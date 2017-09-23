// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/fapply.hpp"

#include <measurement_kit/common.hpp>

static void zero_arguments() {
    REQUIRE(true);
}

static void one_argument(int x) {
    REQUIRE(x == 17);
}

static void two_arguments(int x, int y) {
    REQUIRE(x == 7);
    REQUIRE(y == 11);
}

static void three_arguments(int x, int y, int z) {
    REQUIRE(x == 7);
    REQUIRE(y == 11);
    REQUIRE(z == 0);
}

TEST_CASE("mk::fapply() works as expected with a C function") {
    SECTION("For no arguments") {
        mk::fapply(zero_arguments);
    }

    SECTION("For one argument") {
        mk::fapply(one_argument, 17);
    }

    SECTION("For two arguments") {
        mk::fapply(two_arguments, 7, 11);
    }

    SECTION("For three arguments") {
        mk::fapply(three_arguments, 7, 11, 0);
    }
}

TEST_CASE("mk::fapply() works as expected with a callable") {
    SECTION("For no arguments") {
        mk::fapply([]() { REQUIRE(true); });
    }

    SECTION("For one argument") {
        mk::fapply([](int x) { REQUIRE(x == 17); }, 17);
    }

    SECTION("For two arguments") {
        mk::fapply([](int x, int y) {
            REQUIRE(x == 7);
            REQUIRE(y == 11);
        }, 7, 11);
    }

    SECTION("For three arguments") {
        mk::fapply([](int x, int y, int z) {
            REQUIRE(x == 7);
            REQUIRE(y == 11);
            REQUIRE(z == 0);
        }, 7, 11, 0);
    }
}
