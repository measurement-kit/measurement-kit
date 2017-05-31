// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

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
        auto error = json_parse_and_process("{}", [&](auto json) {
            json.at("foobar");
        });
        REQUIRE(error == JsonKeyError());
    }

    SECTION("In case of domain error") {
        auto error = json_parse_and_process("[]", [&](auto json) {
            json["foobar"];
        });
        REQUIRE(error == JsonDomainError());
    }
}

TEST_CASE("json_parse_process_and_filter_errors works as expected") {
    auto error = json_parse_and_process("{}", [&](auto /*json*/) {
        throw MockedError();
    });
    REQUIRE(error == MockedError());
}
