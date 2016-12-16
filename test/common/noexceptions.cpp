// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common/noexceptions.hpp>
#include <measurement_kit/ext.hpp>

using json = nlohmann::json;

using namespace mk;

TEST_CASE("Try to parse an invalid json") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("this is not a json");
            return value;
    });
    REQUIRE(!value);
    REQUIRE(value.as_error() == JsonParseError());
}

TEST_CASE("Try to get an ivnalid key") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("{\"key\":\"value\"}");
            return value.at("invalid_key");
    });
    REQUIRE(!value);
    REQUIRE_THROWS(*value);
}

TEST_CASE("json_noexcept works: Parse a valid json and get a key") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("{\"key\":\"value\"}");
            return value["key"];
    });
    REQUIRE(!!value);
    REQUIRE(*value == "value");
}
