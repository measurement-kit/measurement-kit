// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/nettests.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace mk;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous tcp-connect test") {
    nettests::TcpConnectTest{}
        .set_options("port", "80")
        .set_input_filepath("test/fixtures/hosts.txt")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .run();
}

TEST_CASE("Asynchronous tcp-connect test") {
    bool done = false;
    nettests::TcpConnectTest{}
        .set_options("port", "80")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_input_filepath("test/fixtures/hosts.txt")
        .start([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

#else
int main() {}
#endif
