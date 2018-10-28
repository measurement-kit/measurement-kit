// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/worker.hpp"
#include "src/libmeasurement_kit/ooni/utils_impl.hpp"

using namespace mk;

TEST_CASE("is_private_ipv4_addr works") {
    REQUIRE(ooni::is_private_ipv4_addr("127.0.0.1") == true);
}

TEST_CASE("extract_html_title works") {
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

TEST_CASE("represent_string works") {
    SECTION("For an ASCII body") {
        std::string s = "an ASCII body";
        report::Entry e = s;
        REQUIRE(ooni::represent_string(s).dump() == e.dump());
    }

    SECTION("For a UTF-8 body") {
        std::vector<uint8_t> v{'a',  'b',  'c', 'd', 'e',
                               0xc3, 0xa8, 'i', 'o', 'u'};
        std::string s{v.begin(), v.end()};
        report::Entry e = s;
        REQUIRE(ooni::represent_string(s).dump() == e.dump());
    }

    SECTION("For a binary body") {
        std::vector<uint8_t> v{0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x02, 0x03};
        std::string s{v.begin(), v.end()};
        REQUIRE(
            ooni::represent_string(s).dump() ==
            (report::Entry{{"format", "base64"}, {"data", "BAMCAQABAgM="}}
                 .dump()));
    }
}

TEST_CASE("find_location() works correctly") {
    ooni::find_location("country.mmdb", "asn.mmdb", {}, Logger::global(),
            [](Error &&err, std::string &&asn, std::string &&cc) {
        REQUIRE(!err);
        REQUIRE(!asn.empty());
        REQUIRE(!cc.empty());
    });
    /*
     * Wait for the default tasks queue to empty, so we exit from the
     * process without still running detached threads and we don't leak
     * memory and, therefore, valgrind memcheck does not fail.
     *
     * See also `test/ooni/orchestrate.cpp`.
     */
    mk::Worker::default_tasks_queue()->wait_empty_();
}
