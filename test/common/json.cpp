// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include <measurement_kit/common/json.hpp>

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("json_process works as expected") {
    SECTION("In case of parse error") {
        auto error = json_process("[", [&](auto) {});
        REQUIRE(error == JsonParseError());
    }

    SECTION("In case of key error") {
        auto error = json_process("{}", [&](auto json) { json.at("foobar"); });
        REQUIRE(error == JsonKeyError());
    }

    SECTION("In case of domain error") {
        auto error = json_process("[]", [&](auto json) { json["foobar"]; });
        REQUIRE(error == JsonDomainError());
    }

    SECTION("When JSON manipulation fails") {
        auto error = json_process("[]", [&](auto json) { json.at("antani"); });
        REQUIRE(error == JsonDomainError());
    }

    SECTION("When an error is thrown by the internal callback") {
        auto error = json_process("{}", [&](auto) { throw MockedError(); });
        REQUIRE(error == MockedError());
    }
}
