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

TEST_CASE("Synchronous http-invalid-request-line test") {
    nettests::HttpInvalidRequestLineTest{}
        .set_options("backend", "http://213.138.109.232/")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .run();
}

TEST_CASE("Synchronous http-invalid-request-line test with HTTP backend") {
    nettests::HttpInvalidRequestLineTest()
        .set_options("backend", "http://data.neubot.org/")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .run();
}

TEST_CASE("Asynchronous http-invalid-request-line test") {
    bool done = false;
    nettests::HttpInvalidRequestLineTest{}
        .set_options("backend", "http://213.138.109.232/")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .start([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

#else
int main() {}
#endif
