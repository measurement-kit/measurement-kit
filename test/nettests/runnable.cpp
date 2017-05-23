// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Make sure that on_entry() works") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    test.reactor = Reactor::global();
    loop_with_initial_event([&]() {
        test.entry_cb = [](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("input") == nullptr));
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
        };
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    });
}

TEST_CASE("Make sure that on_begin() works") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    bool ok = false;
    test.reactor = Reactor::global();
    loop_with_initial_event([&]() {
        test.begin_cb = [&]() {
            ok = true;
        };
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    REQUIRE(ok);
    });
}

TEST_CASE("Make sure that on_end() works") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    int ok = 0;
    test.reactor = Reactor::global();
    loop_with_initial_event([&]() {
        test.end_cbs.push_back([&]() {
            ok += 1;
        });
        test.end_cbs.push_back([&]() {
            ok += 2;
        });
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    REQUIRE(ok == 3);
    });
}

TEST_CASE("Make sure that on_destroy() works") {
    int ok = 0;
    {
        test::nettests::with_runnable([&](nettests::Runnable &test) {
        test.reactor = Reactor::global();
        loop_with_initial_event([&]() {
            test.destroy_cbs.push_back([&]() {
                ok += 1;
            });
            test.destroy_cbs.push_back([&]() {
                ok += 2;
            });
            test.begin([&](Error) {
                test.end([&](Error) {
                    break_loop();
                });
            });
        });
        REQUIRE(ok == 0);
        });
    }
    REQUIRE(ok == 3);
}

TEST_CASE("Ensure we do not save too much information by default") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    test.reactor = Reactor::global();
    test.options["geoip_country_path"] = "GeoIP.dat";
    test.options["geoip_asn_path"] = "GeoIPASNum.dat";
    loop_with_initial_event([&]() {
        test.entry_cb = [](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") != "AS0"));
            REQUIRE((entry.at("probe_cc") != "ZZ"));
            REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
        };
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    });
}

TEST_CASE("Ensure we can save IP address if we want") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    test.reactor = Reactor::global();
    test.options["geoip_country_path"] = "GeoIP.dat";
    test.options["geoip_asn_path"] = "GeoIPASNum.dat";
    test.options["save_real_probe_ip"] = true;
    loop_with_initial_event([&]() {
        test.entry_cb = [](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") != "AS0"));
            REQUIRE((entry.at("probe_cc") != "ZZ"));
            REQUIRE((entry.at("probe_ip") != "127.0.0.1"));
        };
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    });
}

TEST_CASE("Ensure we can avoid saving CC and ASN if we want") {
    test::nettests::with_runnable([](nettests::Runnable &test) {
    test.reactor = Reactor::global();
    test.options["geoip_country_path"] = "GeoIP.dat";
    test.options["geoip_asn_path"] = "GeoIPASNum.dat";
    test.options["save_real_probe_cc"] = false;
    test.options["save_real_probe_asn"] = false;
    loop_with_initial_event([&]() {
        test.entry_cb = [](std::string s) {
            nlohmann::json entry = nlohmann::json::parse(s);
            REQUIRE((entry.at("data_format_version") == "0.2.0"));
            REQUIRE((entry.at("probe_asn") == "AS0"));
            REQUIRE((entry.at("probe_cc") == "ZZ"));
            REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
        };
        test.begin([&](Error) {
            test.end([&](Error) {
                break_loop();
            });
        });
    });
    });
}

TEST_CASE("Make sure that 'randomize_input' works") {

    std::vector<std::string> expect{"torproject.org",
                                    "ooni.nu",
                                    "neubot.org",
                                    "archive.org",
                                    "creativecommons.org",
                                    "cyber.law.harvard.edu",
                                    "duckduckgo.com",
                                    "netflix.com",
                                    "nmap.org",
                                    "www.emule.com"};

    auto run = [&](bool shuffle) -> std::vector<std::string> {

        std::vector<std::string> result;
        test::nettests::with_runnable([&](nettests::Runnable &test) {
        test.reactor = Reactor::make();
        test.input_filepaths.push_back("./test/fixtures/hosts.txt");
        test.options["randomize_input"] = shuffle;
        test.needs_input = true;

        test.reactor->loop_with_initial_event([&]() {
            test.entry_cb = [&](std::string s) {
                nlohmann::json entry = nlohmann::json::parse(s);
                result.push_back(entry["input"]);
            };
            test.begin([&](Error) {
                test.end([&](Error) { test.reactor->break_loop(); });
            });
        });
        });
        return result;
    };

    auto repeat = [&](bool shuffle, int limit) -> int {
        int x = 0;
        /*
         * Since in theory RND_SHUFFLE(vector) may be equal to vector, we
         * measure randomness by counting the number of repetitions for
         * which the shuffle has been found equal to the expected vector.
         */
        while (x++ < limit and run(shuffle) == expect) {
            /* NOTHING */ ;
        }
        return x;
    };


    SECTION("In the common case") {
        // Note: the default should be that input is randomized
        REQUIRE(repeat(true, 8) < 8);
    }

    SECTION("When the user does not want input to be shuffled") {
        REQUIRE(repeat(false, 8) == 9);
    }
}

#else
int main() {}
#endif
