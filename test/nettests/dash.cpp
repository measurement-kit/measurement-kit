// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous dash test") {
    test::nettests::make_test<DashTest>().run();
}

TEST_CASE("Asynchronous dash test") {
    test::nettests::run_async(test::nettests::make_test<DashTest>());
}

#endif
