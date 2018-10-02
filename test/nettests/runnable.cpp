// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/ffi.h>

// TODO(bassosimone): compare options here with with_runnable ones
// TODO(bassosimone): see if we can get rid of with_runnable()

// with_hirl_do_ex runs HIRL with minimal options and invokes |f| on each event
// emitted by the test, thus allowing to perform some basic checks. You can
// set specific options using the |options| argument.
static uint64_t
with_hirl_do_ex(const nlohmann::json &options,
        std::function<bool(const nlohmann::json &)> &&f) noexcept {
    nlohmann::json settings{
            {"name", "HttpInvalidRequestLine"},
            {"options", options},
    };
    mk_unique_task task{mk_nettest_start(settings.dump().c_str())};
    REQUIRE(!!task);
    uint64_t count = 0;
    while (!mk_task_is_done(task.get())) {
        mk_unique_event event{mk_task_wait_for_next_event(task.get())};
        REQUIRE(!!event);
        auto s = mk_event_serialize(event.get());
        //std::clog << "with_hirl_do_ex: " << s << std::endl;  // to debug
        REQUIRE(!!s);
        auto doc = nlohmann::json::parse(s);
        if (f(doc)) {
            REQUIRE(count < UINT64_MAX);
            count += 1;
        }
    }
    REQUIRE(count > 0);
    return count;
}

// with_hirl_do runs HIRL with minimal options and invokes |f| on each event
// emitted by the test, thus allowing to perform some basic checks.
static uint64_t
with_hirl_do(std::function<bool(const nlohmann::json &)> &&f) noexcept {
    return with_hirl_do_ex(nlohmann::json::object(), std::move(f));
}

TEST_CASE("Make sure that the 'measurement' event is okay") {
    with_hirl_do([](const nlohmann::json &doc) noexcept {
        if (doc.at("key") != "measurement") {
            return false;
        }
        auto entry = nlohmann::json::parse(
                doc.at("value").at("json_str").get<std::string>());
        REQUIRE((entry.at("data_format_version") == "0.2.0"));
        REQUIRE((entry.at("input") == nullptr));
        REQUIRE((entry.at("measurement_start_time") != ""));
        REQUIRE((entry.at("probe_asn") == "AS0"));
        REQUIRE((entry.at("probe_cc") == "ZZ"));
        REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
        REQUIRE((entry.at("software_name") == "measurement_kit"));
        REQUIRE((entry.at("software_version") != ""));
        REQUIRE((entry.at("test_keys") != nullptr));
        REQUIRE((entry.at("test_name") == "http_invalid_request_line"));
        REQUIRE((static_cast<double>(entry.at("test_runtime")) > 0.0));
        REQUIRE((entry.at("test_start_time") != ""));
        REQUIRE((entry.at("test_version") == "0.0.3"));
        return true;
    });
}

TEST_CASE("Make sure that the 'status.started' event is okay") {
    auto rv = with_hirl_do([](const nlohmann::json &doc) noexcept {
        return doc.at("key") == "status.started";
    });
    REQUIRE(rv == 1); // Just one event
}

TEST_CASE("Make sure that 'status.end' event is okay") {
    auto rv = with_hirl_do([](const nlohmann::json &doc) noexcept {
        if (doc.at("key") != "status.end") {
            return false;
        }
        auto &v = doc.at("value");
        REQUIRE(v.at("downloaded_kb").get<double>() > 0.0);
        REQUIRE(v.at("uploaded_kb").get<double>() > 0.0);
        REQUIRE(v.at("failure") == "");
        return true;
    });
    REQUIRE(rv == 1); // Just one event
}

TEST_CASE("Ensure we do not save too much information by default") {
    with_hirl_do_ex({
                            {"geoip_country_path", "GeoIP.dat"},
                            {"geoip_asn_path", "GeoIPASNum.dat"},
                    },
            [](const nlohmann::json &doc) noexcept {
                if (doc.at("key") != "measurement") {
                    return false;
                }
                auto entry = nlohmann::json::parse(
                        doc.at("value").at("json_str").get<std::string>());
                REQUIRE((entry.at("probe_asn") != "AS0"));
                REQUIRE((entry.at("probe_cc") != "ZZ"));
                REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
                return true;
            });
}

TEST_CASE("Ensure we can save IP address if we want") {
    with_hirl_do_ex({
                            {"geoip_country_path", "GeoIP.dat"},
                            {"geoip_asn_path", "GeoIPASNum.dat"},
                            {"save_real_probe_ip", true},
                    },
            [](const nlohmann::json &doc) noexcept {
                if (doc.at("key") != "measurement") {
                    return false;
                }
                auto entry = nlohmann::json::parse(
                        doc.at("value").at("json_str").get<std::string>());
                REQUIRE((entry.at("probe_asn") != "AS0"));
                REQUIRE((entry.at("probe_cc") != "ZZ"));
                REQUIRE((entry.at("probe_ip") != "127.0.0.1"));
                return true;
            });
}

TEST_CASE("Ensure we can avoid saving CC and ASN if we want") {
    with_hirl_do_ex({
                            {"geoip_country_path", "GeoIP.dat"},
                            {"geoip_asn_path", "GeoIPASNum.dat"},
                            {"save_real_probe_asn", false},
                            {"save_real_probe_cc", false},
                    },
            [](const nlohmann::json &doc) noexcept {
                if (doc.at("key") != "measurement") {
                    return false;
                }
                auto entry = nlohmann::json::parse(
                        doc.at("value").at("json_str").get<std::string>());
                REQUIRE((entry.at("probe_asn") == "AS0"));
                REQUIRE((entry.at("probe_cc") == "ZZ"));
                REQUIRE((entry.at("probe_ip") == "127.0.0.1"));
                return true;
            });
}

static uint64_t
randomize_input_test_helper(bool randomize_input, const nlohmann::json &inputs,
        std::vector<std::string> *o) noexcept {
    REQUIRE(o != nullptr);
    nlohmann::json settings{
            {"name", "TcpConnect"},
            {"inputs", inputs},
            {"options", {
                                // During #1297, I have experienced a lot
                                // of confusion because this test wasn't
                                // working correctly, but only under Valgrind,
                                // due to race conditions by which the order
                                // with which parallel tests were started was
                                // not the one in which they ended. Avoid this
                                // kind of unpredictable behavior by forcing
                                // no parallelism.
                                {"parallelism", 1},
                                {"randomize_input", randomize_input},
                        }}};
    mk_unique_task task{mk_nettest_start(settings.dump().c_str())};
    REQUIRE(!!task);
    uint64_t count = 0;
    while (!mk_task_is_done(task.get())) {
        mk_unique_event event{mk_task_wait_for_next_event(task.get())};
        REQUIRE(!!event);
        auto s = mk_event_serialize(event.get());
        /*
        std::clog << "randomize_input_test_helper: "
                  << s << std::endl;  // to debug
        */
        REQUIRE(!!s);
        auto doc = nlohmann::json::parse(s);
        if (doc.at("key") != "status.measurement_start") {
            continue;
        }
        REQUIRE(count < UINT64_MAX);
        count += 1;
        auto input = doc.at("value").at("input").get<std::string>();
        o->push_back(input);
    }
    REQUIRE(count > 0);
    return count;
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
        randomize_input_test_helper(shuffle, expect, &result);
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
            /* NOTHING */;
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
