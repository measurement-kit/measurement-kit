// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous dash test") {
    test::nettests::with_test<DashTest>(test::nettests::run_test);
}

#endif
