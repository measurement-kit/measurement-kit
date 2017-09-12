// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/maybe.hpp"

using namespace mk;

TEST_CASE("The Maybe monad works") {
    SECTION("When it is empty") {
        Maybe<int> monad;
        REQUIRE_THROWS(*monad);
        REQUIRE(!monad);
    }

    SECTION("When it is not empty") {
        Maybe<int> monad{17};
        REQUIRE(*monad == 17);
        REQUIRE(!!monad);
    }
}
