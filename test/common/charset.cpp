// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>

TEST_CASE("is_valid_utf8_string works") {

    SECTION("If the input contains no UTF-8") {
        std::string s = "goodbye, cruel world\n";
        REQUIRE(mk::is_valid_utf8_string(s) == mk::NoError());
    }

    SECTION("If the input contains valid UTF-8") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0xc3, 0xbc, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::is_valid_utf8_string(s) == mk::NoError());
    }

    SECTION("If the input contains an illegal sequence") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0xbc, 0xbc, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::is_valid_utf8_string(s) == mk::IllegalSequenceError());
    }

    SECTION("If there is a null byte in the middle") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b',  'y',  'e',
                               ',', ' ', 'c', 'r', 0x00, 0x00, 'e',
                               'l', ' ', 'w', 'o', 'r',  'l',  'd'};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::is_valid_utf8_string(s) == mk::UnexpectedNullByteError());
    }

    SECTION("If the UTF-8 sequence is not complete") {
        std::vector<uint8_t> v{'g', 'o', 'o', 'd', 'b', 'y',
                               'e', ',', ' ', 'c', 'r', 0xc3};
        std::string s{v.begin(), v.end()};
        REQUIRE(mk::is_valid_utf8_string(s) ==
                mk::IncompleteUtf8SequenceError());
    }
}
