// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ooni/whatsapp.hpp"
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Whatsapp and related utility functions") {

    SECTION("Synchronous whatsapp test") {
        test::nettests::with_test<WhatsappTest>(test::nettests::run_test);
    }

    SECTION("can convert ipv4 and ipv6 strings to arrays of bytes") {
        std::vector<uint8_t> ex = {1, 0, 0, 0};
        std::vector<uint8_t> bs = mk::ooni::ip_to_bytes("1.0.0.0");
        REQUIRE(bs == ex);

        ex = {255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bs = mk::ooni::ip_to_bytes("ffff::");
        REQUIRE(bs == ex);
    }

    SECTION("can tell if two ips have the same prefix") {
        // same prefix 1
        std::vector<uint8_t> ip1 = {1, 0, 0, 0};
        std::vector<uint8_t> ip2 = {1, 0, 0, 0};
        ErrorOr<bool> result = mk::ooni::same_pre(ip1, ip2, 32);
        REQUIRE(!!result);
        REQUIRE(result.as_value() == true);

        // same prefix 2
        ip1 = {1, 0, 0, 0};
        ip2 = {1, 0, 0, 255};
        result = mk::ooni::same_pre(ip1, ip2, 24);
        REQUIRE(!!result);
        REQUIRE(result.as_value() == true);

        // different prefix 1
        ip1 = {1, 0, 0, 0};
        ip2 = {2, 0, 0, 0};
        result = mk::ooni::same_pre(ip1, ip2, 32);
        REQUIRE(!!result);
        REQUIRE(result.as_value() == false);

        // different prefix 2
        ip1 = {1, 0, 0, 192};
        ip2 = {1, 0, 0, 129};
        result = mk::ooni::same_pre(ip1, ip2, 26);
        REQUIRE(!!result);
        REQUIRE(result.as_value() == false);
    }

    SECTION("can tell if an ip is within a network") {
        // should be true
        ErrorOr<bool> result = mk::ooni::ip_in_net("10.0.0.1", "10.0.0.0/8");
        REQUIRE(!!result);
        REQUIRE(result.as_value() == true);

        // should be false
        result = mk::ooni::ip_in_net("11.0.0.1", "10.0.0.0/8");
        REQUIRE(!!result);
        REQUIRE(result.as_value() == false);
    }
}
