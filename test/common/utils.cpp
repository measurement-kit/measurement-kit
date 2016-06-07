// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <cctype>
#include <measurement_kit/common.hpp>
#include "src/common/utils.hpp"

TEST_CASE("We are NOT using the default random seed") {
    // Note: the default random generator shall be seeded using 1 unless
    // seeded otherwise, according to Linux and MacOS manpages
    REQUIRE(mk::random_str(16) != "2PN0bdwPY7CA8M06");
}

TEST_CASE("random_within_charset() works with zero length string") {
    REQUIRE_THROWS_AS(mk::random_within_charset("", 16), mk::ValueError);
}

TEST_CASE("random_within_charset() uses all the available charset") {
    REQUIRE(mk::random_within_charset("x", 4) == "xxxx");
}

TEST_CASE("random_printable() generates printable characters") {
    for (auto x : mk::random_str(65536)) {
        REQUIRE((x >= ' ' && x <= '~'));
    }
}

TEST_CASE("random_str() really generates only characters or numbers") {
    auto found_num = false;
    auto found_upper = false;
    auto found_low = false;
    for (auto x : mk::random_str(1024)) {
        if (isdigit(x)) {
            found_num = true;
        } else if (isupper(x)) {
            found_upper = true;
        } else if (islower(x)) {
            found_low = true;
        } else {
            REQUIRE(false);
        }
    }
    REQUIRE(found_num);
    REQUIRE(found_upper);
    REQUIRE(found_low);
}

TEST_CASE("random_str_uppercase() really generates only uppercase") {
    auto found_num = false;
    auto found_upper = false;
    for (auto x : mk::random_str_uppercase(1024)) {
        if (isdigit(x)) {
            found_num = true;
        } else if (isupper(x)) {
            found_upper = true;
        } else {
            REQUIRE(false);
        }
    }
    REQUIRE(found_num);
    REQUIRE(found_upper);
}

TEST_CASE("IPv4 addresses are correctly reversed") {
    REQUIRE(mk::unreverse_ipv4("211.91.192.130.in-addr.arpa") ==
            "130.192.91.211");
    REQUIRE(mk::unreverse_ipv4("4.3.2.1.in-addr.arpa.") ==
            "1.2.3.4");
    REQUIRE(mk::unreverse_ipv4("22.177.3.149.in-addr.arpa") ==
            "149.3.177.22");
}

TEST_CASE("IPv4 addresses cannot contain numbers > 255") {
    REQUIRE(mk::unreverse_ipv4("254.777.254.254.in-addr.arpa") ==
            "");
    REQUIRE(mk::unreverse_ipv4("255.255.255.255.in-addr.arpa") ==
            "255.255.255.255");
}

TEST_CASE("IPv6 addresses are correctly reversed") {
    REQUIRE(mk::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa") == "2001:0db8:0000:0000:0000:0000:0567:89ab");
    REQUIRE(mk::unreverse_ipv6(
                "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa.") == "2001:0db8:0000:0000:0000:0000:0000:0001");
    REQUIRE(mk::unreverse_ipv6(
                "4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.8.0.8.0.2.0.0.4.0.5.4.1.0.0.a."
                "2.ip6.arpa") == "2a00:1450:4002:0808:0000:0000:0000:2004");
}

TEST_CASE("Verify that invalid input is rejected") {

    SECTION("For IPv4") {
        REQUIRE(mk::unreverse_ipv4("") == "");
        // First non number non dot character we break and search in-addr.arpa
        REQUIRE(mk::unreverse_ipv4("foobar") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(mk::unreverse_ipv4("4.3.2.1") == "");
        REQUIRE(mk::unreverse_ipv4("4.3.2.1.") == "");
    }

    SECTION("For IPv6") {
        REQUIRE(mk::unreverse_ipv6("") == "");
        // We encounter a character that is not a dot in position N + 1
        REQUIRE(mk::unreverse_ipv6("e.2.1;d.e.a.d") == "");
        // We encounter a character that is not hex in position N
        REQUIRE(mk::unreverse_ipv6("d.e.a.d.r.e.e.f") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(
            mk::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2") == "");
        REQUIRE(
            mk::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.") == "");
    }
}

TEST_CASE("split(std::string s) works properly in the common case") {
    REQUIRE((mk::split(" 34    43  17 11 ") == std::list<std::string>{
                {"", "34", "43", "17", "11"}
            }));
}

TEST_CASE("split(std::string s) works properly with only one token") {
    REQUIRE((mk::split("34") == std::list<std::string>{
                {"34"}
            }));
}
