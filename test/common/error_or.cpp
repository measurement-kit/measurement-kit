// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

struct Foobar {
    int foo = 17;
    double bar = 3.14;
};

TEST_CASE("The ErrorOr template works as expected when there is no error") {
    ErrorOr<int> eo(0xdeadbeef);
    REQUIRE(static_cast<bool>(eo) == true);
    REQUIRE(eo.as_error() == NoError());
    REQUIRE(*eo == 0xdeadbeef);
}

TEST_CASE("The value enclosed by ErrorOr can be modified") {
    ErrorOr<int> eo(0xdeadbeef);
    *eo = 0xabad1dea;
    REQUIRE(*eo == 0xabad1dea);
}

TEST_CASE("The ErrorOr template works as expected when there is an error") {
    ErrorOr<int> eo{GenericError()};
    REQUIRE(static_cast<bool>(eo) == false);
    REQUIRE(eo.as_error() == GenericError());
    REQUIRE_THROWS_AS(*eo, Error);
}

TEST_CASE("The ErrorOr template works as expected when the empty "
          "constructor is called") {
    ErrorOr<int> eo;
    REQUIRE(static_cast<bool>(eo) == false);
    REQUIRE(eo.as_error() == NotInitializedError());
    REQUIRE_THROWS_AS(*eo, Error);
}

TEST_CASE("One can use arrow operator to access structure wrapped "
          "by ErrorOr template") {
    ErrorOr<Foobar> eo{Foobar{}};
    REQUIRE(eo->foo == 17);
    REQUIRE(eo->bar == 3.14);
}

TEST_CASE("Operator-* throws on error if ErrorOr is not initialized") {
    ErrorOr<int> eo;
    REQUIRE_THROWS_AS(*eo, Error);
}

TEST_CASE("Operator-> throws on error if ErrorOr is not initialized") {
    ErrorOr<Foobar> eo;
    REQUIRE_THROWS_AS(eo->foo = 10, Error);
}

TEST_CASE("Operator-* throws on error if ErrorOr is an error") {
    ErrorOr<int> eo{GenericError()};
    REQUIRE_THROWS_AS(*eo, Error);
}

TEST_CASE("Operator-> throws on error if ErrorOr is an error") {
    ErrorOr<Foobar> eo{GenericError()};
    REQUIRE_THROWS_AS(eo->foo = 10, Error);
}
