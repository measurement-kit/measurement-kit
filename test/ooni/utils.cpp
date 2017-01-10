// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/ooni/utils_impl.hpp"

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("ip lookup works") {
    mk::loop_with_initial_event([]() {
        mk::ooni::ip_lookup([](mk::Error err, std::string) {
            REQUIRE(err == mk::NoError());
            mk::break_loop();
        });
    });
}

#endif

TEST_CASE("geoip works") {
    auto asn = mk::ooni::GeoipCache::global()->resolve_asn(
            "GeoIPASNum.dat",
            "130.192.91.231"
    );
    auto cname = mk::ooni::GeoipCache::global()->resolve_country_name(
            "GeoIP.dat",
            "130.192.91.231"
    );
    auto cc = mk::ooni::GeoipCache::global()->resolve_country_code(
            "GeoIP.dat",
            "130.192.91.231"
    );
    auto city = mk::ooni::GeoipCache::global()->resolve_city_name(
            "GeoLiteCity.dat",
            "130.192.91.231"
    );
    REQUIRE(*asn == std::string{"AS137"});
    REQUIRE(*cc == std::string{"IT"});
    REQUIRE(*cname == std::string{"Italy"});
    REQUIRE(*city == std::string{"Turin"});
}

TEST_CASE("geoip memoization works") {
    mk::ooni::GeoipCache::global()->invalidate(); // Start clean

    // Open more then once. After the first open we should not really open.
    auto gi = mk::ooni::GeoipCache::global()->get(
        "GeoIP.dat");
    bool first_open;

    first_open = true;
    gi = mk::ooni::GeoipCache::global()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    // Repeat two more times to make sure behavior is consistent

    first_open = true;
    gi = mk::ooni::GeoipCache::global()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    first_open = true;
    gi = mk::ooni::GeoipCache::global()->get(
        "GeoIP.dat", first_open);
    REQUIRE(first_open == false);

    // Make sure that, if we change at least one file name, we reopen all

    first_open = false;
    gi = mk::ooni::GeoipCache::global()->get(
        "GeoLiteCity.dat", first_open);
    REQUIRE(first_open == true);

    // Make sure that, if we close, then of course we reopen

    mk::ooni::GeoipCache::global()->invalidate();

    first_open = false;
    gi = mk::ooni::GeoipCache::global()->get(
        "GeoLiteCity.dat", first_open);
    REQUIRE(first_open == true);

}

TEST_CASE("IpLocation::resolve_countr_code() deals with nonexistent database") {
    REQUIRE((mk::ooni::GeoipCache::global()->resolve_country_code(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == mk::ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("IpLocation::resolve_countr_name() deals with nonexistent database") {
    REQUIRE((mk::ooni::GeoipCache::global()->resolve_country_name(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == mk::ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("IpLocation::resolve_asn() deals with nonexistent database") {
    REQUIRE((mk::ooni::GeoipCache::global()->resolve_asn(
                    "invalid.dat", "8.8.8.8"
                ).as_error()
             == mk::ooni::GeoipDatabaseOpenError()));
}

TEST_CASE("is_ip_addr works on ipv4") {
    REQUIRE(mk::ooni::is_ip_addr("127.0.0.1") == true);
}

TEST_CASE("is_ip_addr works on ipv6") {
    REQUIRE(mk::ooni::is_ip_addr("::42") == true);
}

TEST_CASE("is_ip_addr works on hostnames") {
    REQUIRE(mk::ooni::is_ip_addr("example.com") == false);
}

TEST_CASE("is_private_ipv4_addr works") {
    REQUIRE(mk::ooni::is_private_ipv4_addr("127.0.0.1") == true);
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
    REQUIRE(mk::ooni::extract_html_title(body) == "TITLE");
}

TEST_CASE("represent_http_body works") {
    SECTION("For an ASCII body") {
        std::string s = "an ASCII body";
        mk::report::Entry e = s;
        REQUIRE(mk::ooni::represent_http_body(s).dump() == e.dump());
    }

    SECTION("For a UTF-8 body") {
        std::vector<uint8_t> v{'a',  'b',  'c', 'd', 'e',
                               0xc3, 0xa8, 'i', 'o', 'u'};
        std::string s{v.begin(), v.end()};
        mk::report::Entry e = s;
        REQUIRE(mk::ooni::represent_http_body(s).dump() == e.dump());
    }

    SECTION("For a binary body") {
        std::vector<uint8_t> v{0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x02, 0x03};
        std::string s{v.begin(), v.end()};
        REQUIRE(
            mk::ooni::represent_http_body(s).dump() ==
            (mk::report::Entry{{"format", "base64"}, {"data", "BAMCAQABAgM="}}
                 .dump()));
    }
}
