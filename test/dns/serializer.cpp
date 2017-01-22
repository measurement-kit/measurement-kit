// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/serializer_impl.hpp"
#include "../src/libmeasurement_kit/dns/parser.hpp"

using namespace mk;

static int ares_create_query_fail(const char *, int, int, unsigned short,
                                  int, unsigned char **, int *, int) {
    return ARES_EBADNAME;
}

TEST_CASE("The serializer works as expected") {
    ErrorOr<std::vector<uint8_t>> res;

    SECTION("For ares_create_query() failure") {
        res = dns::serialize_impl<ares_create_query_fail>(
            "www.google.com", dns::MK_DNS_TYPE_A, dns::MK_DNS_CLASS_IN, 11, 1,
            Logger::global());
        REQUIRE(!res);
        REQUIRE(res.as_error().code == dns::BadNameError().code);
    }

    SECTION("In the common case") {

        // clang-format off
        static const uint8_t pkt[] = {
            /*-snip--*/ 0xcc, 0xa9, 0x01, 0x00, 0x00, 0x01, /* ........ */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x67, /* .......g */
            0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, /* oogle.co */
            0x6d, 0x00, 0x00, 0x01, 0x00, 0x01              /* m..... */
        };
        // clang-format on

        res = dns::serialize("google.com", dns::MK_DNS_TYPE_A,
                             dns::MK_DNS_CLASS_IN, 0xcca9, 1, Logger::global());
        REQUIRE(!!res);
        std::vector<uint8_t> expect{pkt, pkt + sizeof(pkt) / sizeof(pkt[0])};
        REQUIRE(*res == expect);
    }
}
