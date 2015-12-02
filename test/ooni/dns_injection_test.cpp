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

using namespace measurement_kit::common;
using namespace measurement_kit;

TEST_CASE("Synchronous dns-injection test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    ooni::DnsInjectionTest()
        .set_backend("8.8.8.1:53")
        .set_input_file_path("test/fixtures/hosts.txt")
        .on_log([=](const char *s) { logs->push_back(s); })
        .run();
    for (auto &s : *logs) std::cout << s << "\n";
}

TEST_CASE("Asynchronous dns-injection test") {
    Var<std::list<std::string>> logs(new std::list<std::string>);
    bool done = false;
    ooni::DnsInjectionTest()
        .set_backend("8.8.8.1:53")
        .set_input_file_path("test/fixtures/hosts.txt")
        .on_log([=](const char *s) { logs->push_back(s); })
        .run([&]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
    for (auto &s : *logs) std::cout << s << "\n";
}
