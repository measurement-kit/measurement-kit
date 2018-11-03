// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/maybe.hpp"

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
