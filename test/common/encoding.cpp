// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/encoding.hpp"

#include <resolv.h>

#include <event2/util.h>

#include <measurement_kit/common.hpp>

TEST_CASE("utf8_parse works") {

    SECTION("If the input contains no UTF-8") {
        std::string s = "goodbye, cruel world\n";
        REQUIRE(mk::utf8_parse(s) == mk::NoError());
    }

    SECTION("If the input contains valid UTF-8") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0xc3, 0xbc, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::utf8_parse(s) == mk::NoError());
    }

    SECTION("If the input contains an illegal sequence") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0xbc, 0xbc, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::utf8_parse(s) == mk::IllegalSequenceError());
    }

    SECTION("If there is a null byte in the middle") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0x00, 0x00, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::utf8_parse(s) == mk::UnexpectedNullByteError());
    }

    SECTION("If the UTF-8 sequence is not complete") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b', 'y',
                               'e', ',', ' ', 'c', 'r', 0xc3};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::utf8_parse(s) ==
                mk::IncompleteUtf8SequenceError());
    }
}

TEST_CASE("base64_encode() works as expected") {
    // Note: 'any carnal pleasure' example taken from wikipedia
    //  https://en.wikipedia.org/wiki/Base64

    SECTION("When there are two paddings") {
        REQUIRE(mk::base64_encode("any carnal pleas") ==
                "YW55IGNhcm5hbCBwbGVhcw==");
    }

    SECTION("When there is just one padding") {
        REQUIRE(mk::base64_encode("any carnal pleasu") ==
                "YW55IGNhcm5hbCBwbGVhc3U=");
    }

    SECTION("When there is no padding") {
        REQUIRE(mk::base64_encode("any carnal pleasur") ==
                "YW55IGNhcm5hbCBwbGVhc3Vy");
    }

    SECTION("With random input") {
        for (int i = 0; i < 16; ++i) {
            uint16_t size = 0;
            evutil_secure_rng_get_bytes(&size, sizeof(size));
            if (size == 0) {
                size = 16; /* Avoid using zero as length */
            }
            std::vector<char> vec;
            vec.resize(size);
            evutil_secure_rng_get_bytes(vec.data(), vec.size());
            std::string input(vec.begin(), vec.end());
            std::string result = mk::base64_encode(input);
            std::vector<char> output;
            output.resize(size * 2 /* By excess */);
            /*
             * Q: Why don't you use b64_ntop() in mk::base_encode()?
             * A: To write directly into a string.
             */
            int ctrl = b64_ntop((const uint8_t *)input.data(), input.size(),
                                output.data(), output.size());
            REQUIRE(ctrl > 0);
            // Cast to unsigned type safe because we exclude negative above
            std::string expect{output.data(), (size_t)ctrl};
            REQUIRE(result == expect);
        }
    }
}
