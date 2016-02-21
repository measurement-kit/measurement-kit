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

#include <atomic>
#include <unistd.h>

using namespace mk;

static void run_http_invalid_request_line(Async &async, std::atomic<int> &num) {
    async.run_test(ooni::HttpInvalidRequestLineTest()
                       .set_backend("http://nexa.polito.it/")
                       .on_log([](const char *s) {
                           (void)fprintf(stderr, "test #1: %s\n", s);
                       })
                       .set_verbose()
                       .create_test_(),
                   [&async, &num](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                       if (++num > 16) {
                           return;
                       }
                       run_http_invalid_request_line(async, num);
                   });
}

static void run_dns_injection(Async &async, std::atomic<int> &num) {
    async.run_test(ooni::DnsInjectionTest()
                       .set_backend("8.8.8.8:53")
                       .set_input_file_path("test/fixtures/hosts.txt")
                       .on_log([](const char *s) {
                           (void)fprintf(stderr, "test #3: %s\n", s);
                       })
                       .set_verbose()
                       .create_test_(),
                   [&async, &num](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                       if (++num > 16) {
                           return;
                       }
                       run_dns_injection(async, num);
                   });
}

static void run_tcp_connect(Async &async, std::atomic<int> &num) {
    async.run_test(ooni::TcpConnectTest()
                       .set_port("80")
                       .set_input_file_path("test/fixtures/hosts.txt")
                       .on_log([](const char *s) {
                           (void)fprintf(stderr, "test #4: %s\n", s);
                       })
                       .set_verbose()
                       .create_test_(),
                   [&async, &num](Var<NetTest> test) {
                       mk::debug("test complete: %llu", test->identifier());
                       if (++num > 16) {
                           return;
                       }
                       run_tcp_connect(async, num);
                   });
}

TEST_CASE("The async engine works as expected") {
    set_verbose(1);

    Async async;
    std::atomic<int> num_tests(0);

    // The callback must be called some time in the future because otherwise
    // it would never be fired, since at this time we don't know whether in the
    // other thread the poller was actually already started or not.
    Poller::global()->call_later(5.0, [&async, &num_tests]() {
        // Create tests in temporary void functions to also check that we can
        // create them in functions that later return in real apps
        run_dns_injection(async, num_tests);
        run_http_invalid_request_line(async, num_tests);
        run_http_invalid_request_line(async, num_tests);
        run_tcp_connect(async, num_tests);
    });
    while (num_tests < 16) {
        sleep(5);
    }
}
