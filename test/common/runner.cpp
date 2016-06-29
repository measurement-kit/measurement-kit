// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni.hpp>

#include <unistd.h>

using namespace mk;

static void run_http_invalid_request_line(Runner &runner) {
    runner.run_test(ooni::HttpInvalidRequestLine()
                       .set_options("backend", "http://nexa.polito.it/")
                       .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
                       .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #1: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_dns_injection(Runner &runner) {
    runner.run_test(ooni::DnsInjection()
                       .set_options("backend", "8.8.8.8:53")
                       .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
                       .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
                       .set_input_filepath("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #3: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_tcp_connect(Runner &runner) {
    runner.run_test(ooni::TcpConnect()
                       .set_options("port", "80")
                       .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
                       .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
                       .set_input_filepath("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #4: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

TEST_CASE("The runner engine works as expected") {

    Runner runner;

    for (int i = 0; i < 4; ++i) {
        mk::debug("do another iteration of tests");

        // Create tests in temporary void functions to also check that we can
        // create them in functions that later return in real apps
        run_dns_injection(runner);
        run_http_invalid_request_line(runner);
        run_http_invalid_request_line(runner);
        run_tcp_connect(runner);

        // TODO Need to implement a better sync mechanism?
        while (!runner.empty()) {
            sleep(1);
        }
    }
}

TEST_CASE("The destructor work as expected if the thread was already joined") {
    Runner runner;
    run_dns_injection(runner);
    run_dns_injection(runner);
    while (!runner.empty()) {
        sleep(1);
    }
    runner.break_loop();
    runner.join();
}

TEST_CASE("Nothing strange happens if no thread is bound to Runner") {
    Runner runner;
    REQUIRE(runner.empty()); // Mainly to avoid unused variable warning
}
