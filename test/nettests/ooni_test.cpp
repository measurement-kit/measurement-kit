// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/nettests.hpp>

using namespace mk;

TEST_CASE("Make sure that on_entry() works") {
    nettests::NetTest test;
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
    nettests::NetTest test;
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
    nettests::NetTest test;
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

TEST_CASE("Ensure we do not save too much information by default") {
    nettests::NetTest test;
    test.set_options("geoip_country_path", "GeoIP.dat");
    test.set_options("geoip_asn_path", "GeoIPASNum.dat");
    loop_with_initial_event([&]() {
        test.on_entry([](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") != "AS0"));
            REQUIRE((entry.at("probe_cc") != "ZZ"));
            REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
}

TEST_CASE("Ensure we can save IP address if we want") {
    nettests::NetTest test;
    test.set_options("geoip_country_path", "GeoIP.dat");
    test.set_options("geoip_asn_path", "GeoIPASNum.dat");
    test.set_options("save_real_probe_ip", true);
    loop_with_initial_event([&]() {
        test.on_entry([](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") != "AS0"));
            REQUIRE((entry.at("probe_cc") != "ZZ"));
            REQUIRE((entry.at("probe_ip") != "127.0.0.1"));
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
}

TEST_CASE("Ensure we can avoid saving CC and ASN if we want") {
    nettests::NetTest test;
    test.set_options("geoip_country_path", "GeoIP.dat");
    test.set_options("geoip_asn_path", "GeoIPASNum.dat");
    test.set_options("save_real_probe_cc", false);
    test.set_options("save_real_probe_asn", false);
    loop_with_initial_event([&]() {
        test.on_entry([](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") == "AS0"));
            REQUIRE((entry.at("probe_cc") == "ZZ"));
            REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
        })
        .begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
}

#else
int main() {}
#endif
