// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common/json.hpp>

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("json_process works as expected") {

    SECTION("In case of parse error") {
        auto error = json_process("[", [&](auto /*json*/) {
            /* NOTHING */
        });
        REQUIRE(error == JsonParseError());
    }

    SECTION("In case of key error") {
        auto error = json_process(
              "{}", [&](auto json) { json.at("foobar"); });
        REQUIRE(error == JsonKeyError());
    }

    SECTION("In case of domain error") {
        auto error =
              json_process("[]", [&](auto json) { json["foobar"]; });
        REQUIRE(error == JsonDomainError());
    }
}

TEST_CASE("json_process works as expected") {
    SECTION("When an error is thrown by the internal callback") {
        auto json = Json::array();
        auto error = json_process(
              json, [&](auto /*json*/) { throw MockedError(); });
        REQUIRE(error == MockedError());
    }
    SECTION("When JSON manipulation fails") {
        auto json = Json::array();
        auto error = json_process(
              json, [&](auto json) { json.at("antani"); });
        REQUIRE(error == JsonDomainError());
    }
}

TEST_CASE("json_process works as expected") {
    SECTION("When an error is thrown by the internal callback") {
        auto error = json_process(
              "{}", [&](auto /*json*/) { throw MockedError(); });
        REQUIRE(error == MockedError());
    }
    SECTION("When JSON parsing fails") {
        auto error = json_process(
              "{", [&](auto /*json*/) { /* OKAY */; });
        REQUIRE(error == JsonParseError());
    }
    SECTION("When JSON manipulation fails") {
        auto error = json_process(
              "[]", [&](auto json) { json.at("antani"); });
        REQUIRE(error == JsonDomainError());
    }
}
