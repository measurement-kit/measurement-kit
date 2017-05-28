// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

static void run_http_invalid_request_line(Runner &runner) {
    runner.start_test(test::nettests::make_test<HttpInvalidRequestLineTest>()
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #1: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_dns_injection(Runner &runner) {
    runner.start_test(test::nettests::make_test<DnsInjectionTest>("hosts.txt")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #3: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
                       mk::debug("test complete: %p", (void *)test.get());
                   });
}

static void run_tcp_connect(Runner &runner) {
    runner.start_test(test::nettests::make_test<TcpConnectTest>("hosts.txt")
                       .set_options("port", "80")
                       .on_log([](uint32_t, const char *s) {
                           (void)fprintf(stderr, "test #4: %s\n", s);
                       })
                       .runnable,
                   [](Var<Runnable> test) {
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
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

TEST_CASE("The destructor work as expected if the thread was already joined") {
    Runner runner;
    run_dns_injection(runner);
    run_dns_injection(runner);
    while (!runner.empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    runner.stop();
}

TEST_CASE("Nothing strange happens if no thread is bound to Runner") {
    Runner runner;
    REQUIRE(runner.empty()); // Mainly to avoid unused variable warning
}

#else
int main(){}
#endif
