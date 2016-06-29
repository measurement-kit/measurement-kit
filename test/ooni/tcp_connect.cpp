// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <chrono>
#include <iostream>
#include <list>
#include <measurement_kit/ooni.hpp>
#include <string>
#include <thread>

using namespace mk;

TEST_CASE(
    "The TCP connect test should run with an input file of DNS hostnames") {
    ooni::TcpConnect tcp_connect("test/fixtures/hosts.txt",
                               {
                                   {"port", "80"},
                               });
    loop_with_initial_event_and_connectivity([&]() {
        tcp_connect.begin([&]() { tcp_connect.end([]() { break_loop(); }); });
    });
}

TEST_CASE("The TCP connect test should fail with an invalid dns resolver") {
    ooni::TcpConnect tcp_connect("test/fixtures/hosts.txt",
                               {{"host", "nexacenter.org"},
                                {"port", "80"},
                                {"dns/nameserver", "8.8.8.1"},
                                {"dns/attempts", 1},
                                {"dns/timeout", 0.0001}});
    loop_with_initial_event([&]() {
        tcp_connect.begin([&]() { tcp_connect.end([]() { break_loop(); }); });
    });
}

TEST_CASE("Synchronous tcp-connect test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::TcpConnect()
        .set_options("port", "80")
        .set_input_filepath("test/fixtures/hosts.txt")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Asynchronous tcp-connect test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    bool done = false;
    ooni::TcpConnect()
        .set_options("port", "80")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_input_filepath("test/fixtures/hosts.txt")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Make sure that set_output_path() works") {
    auto instance = ooni::TcpConnect()
                        // Note: must also set valid input file path otherwise
                        // the constructor
                        // called inside create_test_() throws an exception
                        .set_input_filepath("test/fixtures/hosts.txt")
                        .set_output_filepath("foo.txt")
                        .create_test_();
    auto ptr = static_cast<ooni::OoniTest *>(instance.get());
    REQUIRE(ptr->output_filepath == "foo.txt");
}

TEST_CASE("The test should fail with an invalid dns") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::TcpConnect()
        .set_options("port", "80")
        .set_options("dns/nameserver", "8.8.8.1")
        .set_options("dns/attempts", "1")
        .set_options("dns/timeout", "0.001")
        .set_input_filepath("test/fixtures/hosts.txt")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}
