// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Synchronous http-header-field-manipulation test") {
    test::nettests::make_test<HttpHeaderFieldManipulationTest>()
        .set_options("backend", "http://38.107.216.10:80")
        .run();
}

TEST_CASE("Asynchronous http-header-field-manipulation test") {
    test::nettests::run_async(
        test::nettests::make_test<HttpHeaderFieldManipulationTest>()
            .set_options("backend", "http://38.107.216.10:80")
    );
}

#else
int main() {}
#endif
