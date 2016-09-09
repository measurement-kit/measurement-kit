// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/ooni/utils_impl.hpp"

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("ip lookup works") {
    mk::loop_with_initial_event_and_connectivity([]() {
        mk::ooni::ip_lookup([](mk::Error err, std::string) {
            REQUIRE(err == mk::NoError());
            mk::break_loop();
        });
    });
}

#endif

TEST_CASE("geoip works") {
    mk::ErrorOr<json> json = mk::ooni::geoip(
        "8.8.8.8", "GeoIP.dat", "GeoIPASNum.dat",
        "GeoLiteCity.dat");
    REQUIRE(!!json);
    REQUIRE(((*json)["asn"] == std::string{"AS15169"}));
    REQUIRE(((*json)["country_code"] == std::string{"US"}));
    REQUIRE(((*json)["country_name"] == std::string{"United States"}));
    REQUIRE(((*json)["city_name"] == std::string{"Mountain View"}));
}

TEST_CASE("IPLocation::resolve_countr_code() deals with nonexistent database") {
    mk::ooni::IPLocation ipl("invalid.dat", "invalid.dat");
    REQUIRE((ipl.resolve_country_code("8.8.8.8").as_error()
             == mk::ooni::CannotOpenGeoIpCountryDatabaseError()));
}

TEST_CASE("IPLocation::resolve_countr_name() deals with nonexistent database") {
    mk::ooni::IPLocation ipl("invalid.dat", "invalid.dat");
    REQUIRE((ipl.resolve_country_name("8.8.8.8").as_error()
             == mk::ooni::CannotOpenGeoIpCountryDatabaseError()));
}

TEST_CASE("IPLocation::resolve_asn() deals with nonexistent database") {
    mk::ooni::IPLocation ipl("invalid.dat", "invalid.dat");
    REQUIRE((ipl.resolve_asn("8.8.8.8").as_error()
             == mk::ooni::CannotOpenGeoIpAsnDatabaseError()));
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
