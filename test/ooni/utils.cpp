// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/worker.hpp"
#include "src/libmeasurement_kit/ooni/utils_impl.hpp"

using namespace mk;

static void fail(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                 http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                 SharedPtr<http::Response>, int) {
    reactor->call_soon([=]() { callback(MockedError(), nullptr); });
}

static void http_err(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                     http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                     SharedPtr<http::Response>, int) {
    SharedPtr<http::Response> r{new http::Response};
    r->status_code = 500;
    reactor->call_soon([=]() { callback(NoError(), r); });
}

static void re_fail(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                    http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                    SharedPtr<http::Response>, int) {
    SharedPtr<http::Response> r{new http::Response};
    r->status_code = 200;
    r->body = "antani";
    reactor->call_soon([=]() { callback(NoError(), r); });
}

static void no_ip(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                  http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                  SharedPtr<http::Response>, int) {
    SharedPtr<http::Response> r{new http::Response};
    r->status_code = 200;
    r->body = "<Ip>antani</Ip>";
    reactor->call_soon([=]() { callback(NoError(), r); });
}

static void is_v4(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                  http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                  SharedPtr<http::Response>, int) {
    SharedPtr<http::Response> r{new http::Response};
    r->status_code = 200;
    r->body = "<Ip>8.8.8.8</Ip>";
    reactor->call_soon([=]() { callback(NoError(), r); });
}

static void is_v6(std::string, Callback<Error, SharedPtr<http::Response>> callback,
                  http::Headers, Settings, SharedPtr<Reactor> reactor, SharedPtr<Logger>,
                  SharedPtr<http::Response>, int) {
    SharedPtr<http::Response> r{new http::Response};
    r->status_code = 200;
    r->body = "<Ip>fe80::1</Ip>";
    reactor->call_soon([=]() { callback(NoError(), r); });
}

TEST_CASE("ip lookup works") {

    SECTION("is robust to network error") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<fail>([=](Error err, std::string) {
                REQUIRE(err == MockedError());
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

    SECTION("is robust to http error") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<http_err>([=](Error err, std::string) {
                REQUIRE(err == ooni::HttpRequestError());
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

    SECTION("is robust to regex failure error error") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<re_fail>([=](Error err, std::string) {
                REQUIRE(err == ooni::RegexSearchError());
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

    SECTION("is robust to invalid ip addrress in page") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<no_ip>([=](Error err, std::string) {
                REQUIRE(err == ValueError());
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

    SECTION("correctly recognizes ipv4") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<is_v4>([=](Error err, std::string s) {
                REQUIRE(err == NoError());
                REQUIRE(s == "8.8.8.8");
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

    SECTION("correctly recognizes ipv6") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup_impl<is_v6>([=](Error err, std::string s) {
                REQUIRE(err == NoError());
                REQUIRE(s == "fe80::1");
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }

#ifdef ENABLE_INTEGRATION_TESTS
    SECTION("integration test") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::ip_lookup([=](Error err, std::string) {
                REQUIRE(err == NoError());
                reactor->stop();
            }, {}, reactor, Logger::global());
        });
    }
#endif
}

TEST_CASE("geoip works") {
    auto asn = ooni::GeoipCache::thread_local_instance()->resolve_asn(
            "GeoIPASNum.dat",
            "130.192.16.172"
    );
    auto asn_full = ooni::GeoipCache::thread_local_instance()->resolve_asn_full(
            "GeoIPASNum.dat",
            "130.192.16.172"
    );
    auto cname = ooni::GeoipCache::thread_local_instance()->resolve_country_name(
            "GeoIP.dat",
            "130.192.16.172"
    );
    auto cc = ooni::GeoipCache::thread_local_instance()->resolve_country_code(
            "GeoIP.dat",
            "130.192.16.172"
    );
    REQUIRE(*asn == std::string{"AS137"});
    REQUIRE(*asn_full == std::string{"AS137 Consortium GARR"});
    REQUIRE(*cc == std::string{"IT"});
    REQUIRE(*cname == std::string{"Italy"});
}

TEST_CASE("geoip memoization works") {
    ooni::GeoipCache::thread_local_instance()->invalidate(); // Start clean

    // Open more then once. After the first open we should not really open.
    auto gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIP.dat");

    bool first_open = true;
    gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    // Repeat two more times to make sure behavior is consistent

    first_open = true;
    gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    first_open = true;
    gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    // Make sure that, if we change at least one file name, we reopen all

    first_open = false;
    gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIPASNum.dat", first_open);
    REQUIRE(first_open == true);

    // Make sure that, if we close, then of course we reopen

    ooni::GeoipCache::thread_local_instance()->invalidate();

    first_open = false;
    gi = ooni::GeoipCache::thread_local_instance()->get(
        "GeoIPASNum.dat", first_open);
    REQUIRE(first_open == true);

}

TEST_CASE("IpLocation::resolve_countr_code() deals with nonexistent database") {
    REQUIRE((ooni::GeoipCache::thread_local_instance()->resolve_country_code(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("IpLocation::resolve_countr_name() deals with nonexistent database") {
    REQUIRE((ooni::GeoipCache::thread_local_instance()->resolve_country_name(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("IpLocation::resolve_asn() deals with nonexistent database") {
    REQUIRE((ooni::GeoipCache::thread_local_instance()->resolve_asn(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("is_private_ipv4_addr works") {
    REQUIRE(ooni::is_private_ipv4_addr("127.0.0.1") == true);
}

TEST_CASE("extract_html_title works") {
    std::string body = "<html>\n"
        "<head>\n"
        "<meta>\n"
        "<title>TITLE</title>\n"
        "</head>\n"
        "<body>\n"
        "</body>\n"
        "</html>\n";
    REQUIRE(ooni::extract_html_title(body) == "TITLE");
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

#ifdef ENABLE_INTEGRATION_TESTS
TEST_CASE("find_location() works correctly") {
    ooni::find_location("GeoIP.dat", "GeoIPASNum.dat", {}, Logger::global(),
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
#endif
