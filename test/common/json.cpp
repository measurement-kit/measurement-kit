// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/json.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("json_parse_and_process works as expected") {

    SECTION("In case of parse error") {
        auto error = json_parse_and_process("[", [&](auto /*json*/) {
            /* NOTHING */
        });
        REQUIRE(error == JsonParseError());
    }

    SECTION("In case of key error") {
        auto error = json_parse_and_process(
              "{}", [&](auto json) { json.at("foobar"); });
        REQUIRE(error == JsonKeyError());
    }

    SECTION("In case of domain error") {
        auto error =
              json_parse_and_process("[]", [&](auto json) { json["foobar"]; });
        REQUIRE(error == JsonDomainError());
    }
}

TEST_CASE("json_process_and_filter_errors works as expected") {
    SECTION("When an error is thrown by the internal callback") {
        auto json = nlohmann::json::array();
        auto error = json_process_and_filter_errors(
              json, [&](auto /*json*/) { throw MockedError(); });
        REQUIRE(error == MockedError());
    }
    SECTION("When JSON manipulation fails") {
        auto json = nlohmann::json::array();
        auto error = json_process_and_filter_errors(
              json, [&](auto json) { json.at("antani"); });
        REQUIRE(error == JsonDomainError());
    }
}

TEST_CASE("json_parse_process_and_filter_errors works as expected") {
    SECTION("When an error is thrown by the internal callback") {
        auto error = json_parse_process_and_filter_errors(
              "{}", [&](auto /*json*/) { throw MockedError(); });
        REQUIRE(error == MockedError());
    }
    SECTION("When JSON parsing fails") {
        auto error = json_parse_process_and_filter_errors(
              "{", [&](auto /*json*/) { /* OKAY */; });
        REQUIRE(error == JsonParseError());
    }
    SECTION("When JSON manipulation fails") {
        auto error = json_parse_process_and_filter_errors(
              "[]", [&](auto json) { json.at("antani"); });
        REQUIRE(error == JsonDomainError());
    }
}
