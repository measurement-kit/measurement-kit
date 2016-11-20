// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/nettests.hpp>

using namespace mk;

TEST_CASE("Synchronous web connectivity test") {
    set_verbosity(14);
    nettests::WebConnectivityTest()
        .set_options("backend", "https://a.collector.test.ooni.io:4444")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_options("nameserver", "8.8.8.8")
        .set_input_filepath("test/fixtures/urls.txt")
        .run();
}

#else
int main() {}
#endif
