// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

TEST_CASE("IPv4 addresses are correctly reversed") {
    REQUIRE(measurement_kit::unreverse_ipv4("211.91.192.130.in-addr.arpa") ==
            "130.192.91.211");
    REQUIRE(measurement_kit::unreverse_ipv4("4.3.2.1.in-addr.arpa.") ==
            "1.2.3.4");
    REQUIRE(measurement_kit::unreverse_ipv4("22.177.3.149.in-addr.arpa") ==
            "149.3.177.22");
}

TEST_CASE("IPv4 addresses cannot contain numbers > 255") {
    REQUIRE(measurement_kit::unreverse_ipv4("254.777.254.254.in-addr.arpa") ==
            "");
    REQUIRE(measurement_kit::unreverse_ipv4("255.255.255.255.in-addr.arpa") ==
            "255.255.255.255");
}

TEST_CASE("IPv6 addresses are correctly reversed") {
    REQUIRE(measurement_kit::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa") == "2001:0db8:0000:0000:0000:0000:0567:89ab");
    REQUIRE(measurement_kit::unreverse_ipv6(
                "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa.") == "2001:0db8:0000:0000:0000:0000:0000:0001");
    REQUIRE(measurement_kit::unreverse_ipv6(
                "4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.8.0.8.0.2.0.0.4.0.5.4.1.0.0.a."
                "2.ip6.arpa") == "2a00:1450:4002:0808:0000:0000:0000:2004");
}

TEST_CASE("Verify that invalid input is rejected") {

    SECTION("For IPv4") {
        REQUIRE(measurement_kit::unreverse_ipv4("") == "");
        // First non number non dot character we break and search in-addr.arpa
        REQUIRE(measurement_kit::unreverse_ipv4("foobar") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(measurement_kit::unreverse_ipv4("4.3.2.1") == "");
        REQUIRE(measurement_kit::unreverse_ipv4("4.3.2.1.") == "");
    }

    SECTION("For IPv6") {
        REQUIRE(measurement_kit::unreverse_ipv6("") == "");
        // We encounter a character that is not a dot in position N + 1
        REQUIRE(measurement_kit::unreverse_ipv6("e.2.1;d.e.a.d") == "");
        // We encounter a character that is not hex in position N
        REQUIRE(measurement_kit::unreverse_ipv6("d.e.a.d.r.e.e.f") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(
            measurement_kit::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2") == "");
        REQUIRE(
            measurement_kit::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.") == "");
    }
}
