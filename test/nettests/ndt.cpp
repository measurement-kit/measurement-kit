// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous NDT test") {
    test::nettests::with_test<NdtTest>(test::nettests::run_test);
}

#endif
