// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Synchronous web connectivity test") {
    test::nettests::make_test<WebConnectivityTest>("urls.txt")
        .set_options("backend", "https://a.web-connectivity.th.ooni.io:4442")
        .set_options("nameserver", "8.8.8.8")
        .run();
}

TEST_CASE("Asynchronous web-connectivity test") {
    test::nettests::run_async(
        test::nettests::make_test<WebConnectivityTest>("urls.txt")
            .set_options("backend",
                         "https://a.web-connectivity.th.ooni.io:4442")
            .set_options("nameserver", "8.8.8.8")
    );
}

TEST_CASE("Make sure that IP address scrubbing works") {
    auto test = [](std::function<BaseTest(BaseTest)> f,
                   Callback<std::string /*ip*/, std::string /*entry*/> g) {
        std::string probe_ip;
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([&]() {
            ooni::ip_lookup([&](Error err, std::string ip_addr) {
                REQUIRE(!err);
                probe_ip = ip_addr;
                reactor->break_loop();
            });
        });
        REQUIRE(probe_ip != "");
        auto called = 0;
        f(test::nettests::make_test<WebConnectivityTest>("scrub.txt")
              .set_options("backend",
                           "https://a.web-connectivity.th.ooni.io:4442")
              .set_options("nameserver", "8.8.8.8")
              .on_entry([&](std::string entry) {
                  g(probe_ip, entry);
                  called += 1;
              }))
            .run();
        REQUIRE(called == 1);
    };

    SECTION("By default IP is redacted") {
        bool ip_check = false;
        bool redacted_check = false;
        test([](BaseTest test) { return test; },
             [&](std::string ip, std::string entry) {
                 ip_check = (entry.find(ip) == std::string::npos);
                 redacted_check =
                     (entry.find("[REDACTED]") != std::string::npos);
                 REQUIRE(ip_check);
                 REQUIRE(redacted_check);
             });
    }

    SECTION("IP is redacted when its inclusion is NOT requested") {
        /*
         * See above comment
         */
        bool ip_check = false;
        bool redacted_check = false;
        test(
            [](BaseTest test) {
                return test.set_options("save_real_probe_ip", false);
            },
            [&](std::string ip, std::string entry) {
                 ip_check = (entry.find(ip) == std::string::npos);
                 redacted_check =
                     (entry.find("[REDACTED]") != std::string::npos);
                 REQUIRE(ip_check);
                 REQUIRE(redacted_check);
            });
    }

    SECTION("IP is NOT redacted when its inclusion is requested") {
        /*
         * See above comment
         */
        bool ip_check = false;
        bool redacted_check = false;
        test(
            [](BaseTest test) {
                return test.set_options("save_real_probe_ip", true);
            },
            [&](std::string ip, std::string entry) {
                 ip_check = (entry.find(ip) != std::string::npos);
                 redacted_check =
                     (entry.find("[REDACTED]") == std::string::npos);
                 REQUIRE(ip_check);
                 REQUIRE(redacted_check);
            });
    }
}

#else
int main() {}
#endif
