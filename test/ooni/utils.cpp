// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/utils_impl.hpp"
#include "src/ooni/utils.hpp"

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
        "8.8.8.8", "test/fixtures/GeoIP.dat", "test/fixtures/GeoIPASNum.dat");
    REQUIRE(!!json);
    REQUIRE(((*json)["asn"] == std::string{"AS15169"}));
    REQUIRE(((*json)["country_code"] == std::string{"US"}));
    REQUIRE(((*json)["country_name"] == std::string{"United States"}));
}

TEST_CASE("geoip returns an error if it can't open the country database") {
    mk::ErrorOr<json> json = mk::ooni::geoip(
        "8.8.8.8", "test/fixtures/GeoIPinvalid.dat", "invalid_path.dat");
    REQUIRE(json.as_error() == mk::ooni::CannotOpenGeoIpCountryDatabase());
}

TEST_CASE("geoip returns an error if it can't open the asn database") {
    mk::ErrorOr<json> json = mk::ooni::geoip(
        "8.8.8.8", "test/fixtures/GeoIPinvalid.dat", "invalid_path.dat");
    REQUIRE(json.as_error() == mk::ooni::CannotOpenGeoIpAsnDatabase());
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
