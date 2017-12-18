// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Synchronous dpi_fragment test") {
    test::nettests::with_test<DpiFragmentTest>(test::nettests::run_test);
}

#else
int main() {}
#endif
