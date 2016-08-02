// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/ooni.hpp>

using namespace mk;

TEST_CASE("Make sure that on_entry() works") {
    ooni::OoniTest test;
    loop_with_initial_event([&]() {
        test.on_entry([](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("input") == ""));
            REQUIRE((entry.at("measurement_start_time") != ""));
            REQUIRE((entry.at("probe_asn") == "AS0"));
            REQUIRE((entry.at("probe_cc") == "ZZ"));
            REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
            REQUIRE((entry.at("software_name") == "measurement_kit"));
            REQUIRE((entry.at("software_version") != ""));
            REQUIRE((entry.at("test_keys") != nullptr));
            REQUIRE((entry.at("test_name") == "ooni_test"));
            REQUIRE((static_cast<double>(entry.at("test_runtime")) > 0.0));
            REQUIRE((entry.at("test_start_time") != ""));
            REQUIRE((entry.at("test_version") != ""));
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
}

TEST_CASE("Make sure that on_begin() works") {
    bool ok = false;
    ooni::OoniTest test;
    loop_with_initial_event([&]() {
        test.on_begin([&]() {
            ok = true;
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    REQUIRE(ok);
}

TEST_CASE("Make sure that on_end() works") {
    bool ok = false;
    ooni::OoniTest test;
    loop_with_initial_event([&]() {
        test.on_end([&]() {
            ok = true;
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    REQUIRE(ok);
}

#else
int main() {}
#endif
