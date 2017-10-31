// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/fcdr.hpp"
#include <measurement_kit/common.hpp>

TEST_CASE("mk::fcdr() works as expected") {
    SECTION("For one-element tuples") {
        REQUIRE(mk::fcdr(std::make_tuple(1)) == std::make_tuple());
    }

    SECTION("For two-element tuples") {
        REQUIRE(mk::fcdr(std::make_tuple(1, 2)) == std::make_tuple(2));
    }

    SECTION("For three-element tuples") {
        REQUIRE(mk::fcdr(std::make_tuple(1, 2, 3)) == std::make_tuple(2, 3));
    }
}
