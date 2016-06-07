// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/ooni_test_impl.hpp"
#include <chrono>
#include <iostream>
#include <list>
#include <measurement_kit/ooni.hpp>
#include <string>
#include <thread>

using namespace mk;

TEST_CASE("Synchronous dns-injection test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::DnsInjectionTest()
        .set_options("backend", "8.8.8.1:53")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_options("dns/timeout", "0.1")
        .set_input_file_path("test/fixtures/hosts.txt")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Asynchronous dns-injection test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    bool done = false;
    ooni::DnsInjectionTest()
        .set_options("backend", "8.8.8.1:53")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_options("dns/timeout", "0.1")
        .set_input_file_path("test/fixtures/hosts.txt")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run([&]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Make sure that set_output_path() works") {
    auto instance = ooni::DnsInjectionTest()
                        // Note: must also set valid input file path otherwise
                        // the constructor
                        // called inside create_test_() throws an exception
                        .set_input_file_path("test/fixtures/hosts.txt")
                        .set_output_file_path("foo.txt")
                        .create_test_();
    auto ptr = static_cast<ooni::OoniTestImpl *>(instance.get());
    REQUIRE(ptr->get_output_file_path() == "foo.txt");
}

TEST_CASE("Make sure that default get_output_path() is nonempty") {
    auto instance = ooni::DnsInjectionTest()
                        // Note: must also set valid input file path otherwise
                        // the constructor
                        // called inside create_test_() throws an exception
                        .set_input_file_path("test/fixtures/hosts.txt")
                        .create_test_();
    auto ptr = static_cast<ooni::OoniTestImpl *>(instance.get());
    REQUIRE(ptr->get_output_file_path() != "");
}
