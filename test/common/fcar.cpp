// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/fcar.hpp"

#include <measurement_kit/common.hpp>

TEST_CASE("mk::fcar() works as expected") {
    SECTION("For one-element tuples") {
        REQUIRE(mk::fcar(std::make_tuple(1)) == 1);
    }

    SECTION("For two-element tuples") {
        REQUIRE(mk::fcar(std::make_tuple(1, 2)) == 1);
    }

    SECTION("For three-element tuples") {
        REQUIRE(mk::fcar(std::make_tuple(1, 2, 3)) == 1);
    }
}
