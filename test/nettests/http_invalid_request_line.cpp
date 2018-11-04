// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous http-invalid-request-line test") {
    test::nettests::with_test<HttpInvalidRequestLineTest>(
          test::nettests::run_test);
}

TEST_CASE("Synchronous http-invalid-request-line test with HTTP backend") {
    test::nettests::with_test<HttpInvalidRequestLineTest>([](BaseTest &test) {
        test.set_option("backend", "http://data.neubot.org/").run();
    });
}
