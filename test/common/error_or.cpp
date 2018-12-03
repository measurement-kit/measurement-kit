// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

struct Foobar {
    int foo = 17;
    double bar = 3.14;
};

TEST_CASE("The ErrorOr template works as expected when there is no error") {
    ErrorOr<unsigned long> eo{NoError(), 0xdeadbeef};
    REQUIRE(static_cast<bool>(eo) == true);
    REQUIRE(eo.as_error() == NoError());
    REQUIRE(*eo == 0xdeadbeef);
}

TEST_CASE("The value enclosed by ErrorOr can be modified") {
    ErrorOr<unsigned long> eo{NoError(), 0xdeadbeef};
    *eo = 0xabad1dea;
    REQUIRE(*eo == 0xabad1dea);
}

TEST_CASE("The value enclosed by ErrorOr can be extracted") {
    ErrorOr<std::shared_ptr<Foobar>> eo{NoError(), std::make_shared<Foobar>()};
    std::shared_ptr<Foobar> foobar;
    std::swap(foobar, eo.as_value());
    REQUIRE(!eo.as_value());
}

TEST_CASE("The ErrorOr template works as expected when there is an error") {
    ErrorOr<int> eo{GenericError(), {}};
    REQUIRE(static_cast<bool>(eo) == false);
    REQUIRE(eo.as_error() == GenericError());
    REQUIRE_THROWS_AS(*eo, std::runtime_error);
}

TEST_CASE("The ErrorOr template works as expected when the empty "
          "constructor is called") {
    ErrorOr<int> eo;
    REQUIRE(static_cast<bool>(eo) == false);
    REQUIRE(eo.as_error() == NotInitializedError());
    REQUIRE_THROWS_AS(*eo, std::runtime_error);
}

TEST_CASE("One can use arrow operator to access structure wrapped "
          "by ErrorOr template") {
    ErrorOr<Foobar> eo{NoError(), Foobar{}};
    REQUIRE(eo->foo == 17);
    REQUIRE(eo->bar == 3.14);
}

TEST_CASE("Operator-* throws on error if ErrorOr is not initialized") {
    ErrorOr<int> eo;
    REQUIRE_THROWS_AS(*eo, std::runtime_error);
}

TEST_CASE("Operator-> throws on error if ErrorOr is not initialized") {
    ErrorOr<Foobar> eo;
    REQUIRE_THROWS_AS(eo->foo = 10, std::runtime_error);
}

TEST_CASE("Operator-* throws on error if ErrorOr is an error") {
    ErrorOr<int> eo{GenericError(), {}};
    REQUIRE_THROWS_AS(*eo, std::runtime_error);
}

TEST_CASE("Operator-> throws on error if ErrorOr is an error") {
    ErrorOr<Foobar> eo{GenericError(), {}};
    REQUIRE_THROWS_AS(eo->foo = 10, std::runtime_error);
}
