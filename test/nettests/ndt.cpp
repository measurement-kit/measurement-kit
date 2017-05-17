// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous NDT test") {
    test::nettests::make_test<NdtTest>()
        .run();
}

TEST_CASE("Asynchronous NDT test") {
    test::nettests::run_async(
        test::nettests::make_test<NdtTest>()
    );
}

#else
int main(){}
#endif
