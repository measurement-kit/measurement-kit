// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <chrono>
#include <iostream>
#include <list>
#include <measurement_kit/ooni.hpp>
#include <thread>

using namespace mk;

TEST_CASE("The HTTP Invalid Request Line test should run") {
    Settings options;
    options["backend"] = "http://213.138.109.232/";
    ooni::HttpInvalidRequestLine http_invalid_request_line(options);
    loop_with_initial_event_and_connectivity([&]() {
        http_invalid_request_line.begin(
            [&]() { http_invalid_request_line.end([]() { break_loop(); }); });
    });
}

TEST_CASE(
    "The HTTP Invalid Request Line can manage a failure while connecting") {
    Settings options;
    options["backend"] = "http://213.138.109.232/";
    options["dns/nameserver"] = "8.8.8.1";
    options["dns/timeout"] = 0.1;
    ooni::HttpInvalidRequestLine http_invalid_request_line(options);
    loop_with_initial_event_and_connectivity([&]() {
        http_invalid_request_line.begin([&]() {
            http_invalid_request_line.end([]() { break_loop(); });
        });
    });
}

TEST_CASE("Synchronous http-invalid-request-line test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::HttpInvalidRequestLine()
        .set_options("backend", "http://213.138.109.232/")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Synchronous http-invalid-request-line test with HTTP backend") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::HttpInvalidRequestLine()
        .set_options("backend",
                     "http://data.neubot.org/") // Let's troll Davide!
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Asynchronous http-invalid-request-line test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    bool done = false;
    ooni::HttpInvalidRequestLine()
        .set_options("backend", "http://213.138.109.232/")
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Make sure that set_output_path() works") {
    auto instance = ooni::HttpInvalidRequestLine()
                        .set_output_filepath("foo.txt")
                        .create_test_();
    auto ptr = static_cast<ooni::OoniTest *>(instance.get());
    REQUIRE(ptr->output_filepath == "foo.txt");
}

TEST_CASE("Make sure that it can pass options to the other levels") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::HttpInvalidRequestLine()
        .set_options("backend", "http://nexacenter.org")
        .set_options("dns/nameserver", "8.8.8.1")
        .set_options("dns/timeout", "0.1")
        .set_options("dns/attempts", "1")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}

TEST_CASE("Make sure that the test can deal with an invalid backend") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::HttpInvalidRequestLine()
        .set_options("backend", "nexacenter.org")
        .on_log([=](uint32_t, const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs)
        std::cout << s << "\n";
}
