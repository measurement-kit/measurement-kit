// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous http-invalid-request-line test") {
    test::nettests::make_test<HttpInvalidRequestLineTest>()
        .run();
}

TEST_CASE("Synchronous http-invalid-request-line test with HTTP backend") {
    test::nettests::make_test<HttpInvalidRequestLineTest>()
        .set_options("backend", "http://data.neubot.org/")
        .run();
}

#else
int main() {}
#endif
