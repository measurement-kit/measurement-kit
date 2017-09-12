// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common/detail/sandbox.hpp>

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("The sandbox_for_errors works") {
    SECTION("When no error is thrown") {
        auto err = sandbox_for_errors([&]() {
            /* NOTHING */
        });
        REQUIRE(err == NoError());
    }

    SECTION("When an error is thrown") {
        auto err = sandbox_for_errors([&]() {
            throw MockedError();
        });
        REQUIRE(err == MockedError());
    }
}

TEST_CASE("The sandbox_for_exceptions works") {
    SECTION("When no exception is thrown") {
        auto maybe_exc = sandbox_for_exceptions([&]() {
            /* NOTHING */
        });
        REQUIRE(!maybe_exc);
    }

    SECTION("When an exception is thrown") {
        auto maybe_exc = sandbox_for_exceptions([&]() {
            throw std::runtime_error("antani");
        });
        REQUIRE(!!maybe_exc);
    }

    SECTION("When an error is thrown") {
        auto maybe_exc = sandbox_for_exceptions([&]() {
            throw MockedError();
        });
        REQUIRE(!!maybe_exc);
    }
}
