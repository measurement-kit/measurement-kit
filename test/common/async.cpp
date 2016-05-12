// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/async.cpp's
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni.hpp>

#include <unistd.h>

using namespace mk;

static void run_http_invalid_request_line(Async &async) {
    async.run_test(ooni::HttpInvalidRequestLineTest()
                       .set_options("backend", "http://nexa.polito.it/")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #1: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                   });
}

static void run_dns_injection(Async &async) {
    async.run_test(ooni::DnsInjectionTest()
                       .set_options("backend", "8.8.8.8:53")
                       .set_input_file_path("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #3: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                   });
}

static void run_tcp_connect(Async &async) {
    async.run_test(ooni::TcpConnectTest()
                       .set_options("port", "80")
                       .set_input_file_path("test/fixtures/hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #4: %s\n", s);
                       })
                       .create_test_(),
                   [](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                   });
}

TEST_CASE("The async engine works as expected") {

    Async async;

    for (int i = 0; i < 4; ++i) {
        mk::debug("do another iteration of tests");

        // Create tests in temporary void functions to also check that we can
        // create them in functions that later return in real apps
        run_dns_injection(async);
        run_http_invalid_request_line(async);
        run_http_invalid_request_line(async);
        run_tcp_connect(async);

        // TODO Need to implement a better sync mechanism?
        while (!async.empty()) {
            sleep(1);
        }
    }
}

TEST_CASE("The destructor work as expected if the thread was already joined") {
    Async async;
    run_dns_injection(async);
    run_dns_injection(async);
    while (!async.empty()) {
        sleep(1);
    }
    async.break_loop();
    async.join();
}

TEST_CASE("Nothing strange happens if no thread is bound to Async") {
    Async async;
    REQUIRE(async.empty()); // Mainly to avoid unused variable warning
}

