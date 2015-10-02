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

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

static void run_http_invalid_request_line(Async &async) {
    auto test = SharedPointer<HTTPInvalidRequestLine>(
        new HTTPInvalidRequestLine(Settings{
            {"backend", "http://nexa.polito.it/"},
        })
    );
    test->set_verbose(1);
    test->on_log([](const char *s) {
        (void) fprintf(stderr, "test #1: %s\n", s);
    });
    measurement_kit::debug("test created: %llu", test->identifier());
    async.run_test(test, [](SharedPointer<common::NetTest> test) {
        measurement_kit::debug("test complete: %llu", test->identifier());
    });
}

static void run_dns_injection(Async& async) {
    auto test = SharedPointer<DNSInjection>(
        new DNSInjection("test/fixtures/hosts.txt", Settings{
            {"nameserver", "8.8.8.8:53"},
        })
    );
    test->set_verbose(1);
    test->on_log([](const char *s) {
        (void) fprintf(stderr, "test #3: %s\n", s);
    });
    measurement_kit::debug("test created: %llu", test->identifier());
    async.run_test(test, [](SharedPointer<common::NetTest> test) {
        measurement_kit::debug("test complete: %llu", test->identifier());
    });
}

static void run_tcp_connect(Async& async) {
    auto test = SharedPointer<TCPConnect>(
        new TCPConnect("test/fixtures/hosts.txt", Settings{
            {"port", "80"},
        })
    );
    test->set_verbose(1);
    test->on_log([](const char *s) {
        (void) fprintf(stderr, "test #4: %s\n", s);
    });
    measurement_kit::debug("test created: %llu", test->identifier());
    async.run_test(test, [](SharedPointer<common::NetTest> test) {
        measurement_kit::debug("test complete: %llu", test->identifier());
    });
}

TEST_CASE("The async engine works as expected") {

    measurement_kit::set_verbose(1);
    Async async;

    for (int i = 0; i < 4; ++i) {
        measurement_kit::debug("do another iteration of tests");

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
