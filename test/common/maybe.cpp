// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("The maybe template works as expected when there is no error") {
    Maybe<int> maybe(0xdeadbeef);
    REQUIRE(static_cast<bool>(maybe) == true);
    REQUIRE(maybe.as_error() == NoError());
    REQUIRE(maybe.as_value() == 0xdeadbeef);
}

TEST_CASE("The maybe template works as expected when there is an error") {
    Maybe<int> maybe(GenericError(), 0);
    REQUIRE(static_cast<bool>(maybe) == false);
    REQUIRE(maybe.as_error() == GenericError());
    REQUIRE_THROWS_AS(maybe.as_value(), Error);
}

TEST_CASE("The maybe template works as expected when the empty constructor is called") {
    Maybe<int> maybe;
    REQUIRE(static_cast<bool>(maybe) == false);
    REQUIRE(maybe.as_error() == MaybeNotInitializedError());
    REQUIRE_THROWS_AS(maybe.as_value(), Error);
}
