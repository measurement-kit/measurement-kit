// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/nettests.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace mk;

TEST_CASE("Synchronous http-header-field-manipulation test") {
    nettests::HeaderFieldManipulationTest{}
//        .set_options("backend", "https://a.collector.test.ooni.io:4444")
//        .set_options("geoip_country_path", "GeoIP.dat")
//        .set_options("geoip_asn_path", "GeoIPASNum.dat")
//        .set_options("nameserver", "8.8.8.8")
//        .set_input_filepath("test/fixtures/urls.txt")
        .run();
}

TEST_CASE("Asynchronous http-header-field-manipulation test") {
    bool done = false;
    nettests::HeaderFieldManipulationTest{}
//        .set_options("backend", "https://a.collector.test.ooni.io:4444")
//        .set_options("geoip_country_path", "GeoIP.dat")
//        .set_options("geoip_asn_path", "GeoIPASNum.dat")
//        .set_options("nameserver", "8.8.8.8")
//        .set_input_filepath("test/fixtures/urls.txt")
        .start([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

#else
int main() {}
#endif
