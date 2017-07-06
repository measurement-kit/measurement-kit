// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"
#include "private/ooni/captive_portal.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;
using namespace mk::ooni;

TEST_CASE("Synchronous captive portal test") {
    test::nettests::make_test<CaptivePortalTest>()
        .run();
}

TEST_CASE("Asynchronous captive portal test") {
    test::nettests::run_async(
        test::nettests::make_test<CaptivePortalTest>()
    );
}

#else
int main() {}
#endif
