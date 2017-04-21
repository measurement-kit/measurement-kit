// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common/noexceptions.hpp>
#include <measurement_kit/ext.hpp>

using json = nlohmann::json;

using namespace mk;

TEST_CASE("no exceptions works") {
    auto value = noexceptions<int>([]() {
        throw GenericError();
        return 1;
    });
    REQUIRE(!value);
    REQUIRE(value.as_error() == GenericError());
}

TEST_CASE("test it with json") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("this is not a json");
            return value;
    });
    REQUIRE(!value);
    REQUIRE(value.as_error() == JsonParseError());
}


TEST_CASE("test it with json with valid fields") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("{\"key\":\"value\"}");
            return value["key"];
    });
    REQUIRE(*value == "value");
}

TEST_CASE("test it with json with invalid fields") {
    auto value = json_noexcept<json>([]() {
            auto value = json::parse("{\"key\":\"value\"}");
            return value.at("invalid_key");
    });
    REQUIRE(!value);
    // you should handle the fact that you receive an error
    // but, if you try to access you thrown the previous error
    REQUIRE_THROWS(*value);
}
