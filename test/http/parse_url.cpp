// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/http/http.hpp"

using namespace mk;

TEST_CASE("Works as expected for simple URL") {
    auto url = http::parse_url("http://www.kernel.org/");
    REQUIRE(url.schema == "http");
    REQUIRE(url.address == "www.kernel.org");
    REQUIRE(url.port == 80);
    REQUIRE(url.path == "/");
    REQUIRE(url.query == "");
    REQUIRE(url.pathquery == "/");
}

TEST_CASE("Works as expected for more complex case") {
    auto url = http::parse_url("https://www.kernel.org:54321/abc?foobar");
    REQUIRE(url.schema == "https");
    REQUIRE(url.address == "www.kernel.org");
    REQUIRE(url.port == 54321);
    REQUIRE(url.path == "/abc");
    REQUIRE(url.query == "foobar");
    REQUIRE(url.pathquery == "/abc?foobar");
}

TEST_CASE("Recognizes wrong ports") {
    SECTION("Negative port") {
        REQUIRE_THROWS_AS(
            http::parse_url("https://www.kernel.org:-4/abc?foobar"), Error);
    }

    SECTION("Too large port") {
        REQUIRE_THROWS_AS(
            http::parse_url("https://www.kernel.org:65537/abc?foobar"), Error);
    }
}
