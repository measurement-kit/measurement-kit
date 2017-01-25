// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;

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

#else
int main() {}
#endif
