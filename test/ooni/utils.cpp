// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/utils.hpp"

TEST_CASE("ip lookup works") {
    mk::loop_with_initial_event_and_connectivity([]() {
        mk::ooni::ip_lookup([](mk::Error err, std::string) {
            REQUIRE(err == mk::NoError());
            mk::break_loop();
        });
    });
}

TEST_CASE("geoip works") {
    std::string resolved = "{\"asn\":\"AS15169 Google "
                           "Inc.\",\"country_code\":\"USA\",\"country_name\":"
                           "\"United States\"}";
    mk::ErrorOr<json> json = mk::ooni::geoip(
        "8.8.8.8", "test/fixtures/GeoIP.dat", "test/fixtures/GeoIPASNum.dat");
    REQUIRE(json);
    REQUIRE(resolved == json->dump());
}
