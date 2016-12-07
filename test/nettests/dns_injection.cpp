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

TEST_CASE("Synchronous dns-injection test") {
    nettests::DnsInjectionTest{}
        .set_options("backend", "8.8.8.1:53")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_options("dns/timeout", "0.1")
        .set_input_filepath("test/fixtures/hosts.txt")
        .run();
}

TEST_CASE("Asynchronous dns-injection test") {
    bool done = false;
    nettests::DnsInjectionTest{}
        .set_options("backend", "8.8.8.1:53")
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_options("dns/timeout", "0.1")
        .set_input_filepath("test/fixtures/hosts.txt")
        .start([&]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

#endif

// Note: we don't repeat this test for every type of nettest
TEST_CASE("Make sure that set_output_filepath() works") {
    /*
       Note: must also set valid input file path otherwise the constructor
       called inside create_test_() throws an exception
    */
    auto runnable = nettests::DnsInjectionTest{}
                        .set_input_filepath("test/fixtures/hosts.txt")
                        .set_output_filepath("foo.txt")
                        .runnable;
    REQUIRE(runnable->output_filepath == "foo.txt");
}
