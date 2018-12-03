// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/common/worker.hpp"
#include "src/libmeasurement_kit/ooni/utils_impl.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"

#include <sstream>

using namespace mk;

TEST_CASE("is_private_ipv4_addr works") {
    REQUIRE(ooni::is_private_ipv4_addr("127.0.0.1") == true);
}

TEST_CASE("extract_html_title works") {
    SECTION("For a simple string") {
        std::string body = "<html>\n"
                           "<head>\n"
                           "<meta>\n"
                           "<title>TiTLE</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "</body>\n"
                           "</html>\n";
        REQUIRE(ooni::extract_html_title(body) == "TiTLE");
    }
    SECTION("For a very long string") {
        std::stringstream ss;
        ss << "<html><head><meta><title>";
        ss << random_printable(1 << 25);
        ss << "</title></head></body></html>";
        // Note: we expect the title to be empty because the regular
        // expression is bounded, i.e. it does not look for titles
        // beyond a specific length. (We should probably parse bodies
        // in a test helper rather than in MK but, still, at least
        // with this fix we don't overflow the stack.)
        REQUIRE(ooni::extract_html_title(ss.str()) == "");
    }
}

TEST_CASE("represent_string works") {
    SECTION("For an ASCII body") {
        std::string s = "an ASCII body";
        nlohmann::json e = s;
        REQUIRE(ooni::represent_string(s).dump() == e.dump());
    }

    SECTION("For a UTF-8 body") {
        std::vector<uint8_t> v{'a',  'b',  'c', 'd', 'e',
                               0xc3, 0xa8, 'i', 'o', 'u'};
        std::string s{v.begin(), v.end()};
        nlohmann::json e = s;
        REQUIRE(ooni::represent_string(s).dump() == e.dump());
    }

    SECTION("For a binary body") {
        // Note: the following MUST be invalid UTF-8. Otherwise it can still
        // be converted as a UTF-8 string. The 0xbc 0bc sequence is invalid in
        // UTF-8, hence we'll expect the result to be base64-ed.
        std::vector<uint8_t> v{0x04, 0x03, 0xbc, 0xbc, 0x00, 0x01, 0x02, 0x03};
        std::string s{v.begin(), v.end()};
        REQUIRE(
            ooni::represent_string(s).dump() ==
            (nlohmann::json{{"format", "base64"}, {"data", "BAO8vAABAgM="}}
                 .dump()));
    }
}
