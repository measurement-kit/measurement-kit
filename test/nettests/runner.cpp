// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/nettests.hpp>

#include <unistd.h>

using namespace mk;
using namespace mk::nettests;

static void run_http_invalid_request_line(nettests::Runner &runner) {
    runner.run_test(nettests::HttpInvalidRequestLine()
                       .set_options("backend", "http://nexa.polito.it/")
                       .set_options("geoip_country_path", "GeoIP.dat")
                       .set_options("geoip_asn_path", "GeoIPASNum.dat")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #1: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_dns_injection(nettests::Runner &runner) {
    runner.run_test(nettests::DnsInjection()
                       .set_options("backend", "8.8.8.8:53")
                       .set_options("geoip_country_path", "GeoIP.dat")
                       .set_options("geoip_asn_path", "GeoIPASNum.dat")
                       .set_input_filepath("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #3: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_tcp_connect(nettests::Runner &runner) {
    runner.run_test(nettests::TcpConnect()
                       .set_options("port", "80")
                       .set_options("geoip_country_path", "GeoIP.dat")
                       .set_options("geoip_asn_path", "GeoIPASNum.dat")
                       .set_input_filepath("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #4: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

TEST_CASE("The runner engine works as expected") {

    nettests::Runner runner;

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
    nettests::Runner runner;
    run_dns_injection(runner);
    run_dns_injection(runner);
    while (!runner.empty()) {
        sleep(1);
    }
    runner.break_loop_();
    runner.join_();
}

TEST_CASE("Nothing strange happens if no thread is bound to Runner") {
    nettests::Runner runner;
    REQUIRE(runner.empty()); // Mainly to avoid unused variable warning
}

#else
int main(){}
#endif
