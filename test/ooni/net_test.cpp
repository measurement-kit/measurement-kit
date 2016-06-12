// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/ooni.hpp>

using namespace mk::ooni;
using namespace mk;

TEST_CASE("The NetTest should callback when it has finished running") {
    mk::ooni::OoniTest test("");
    // Note: connectivity not needed here
    loop_with_initial_event([&]() {
        test.begin([&]() { test.end([]() { break_loop(); }); });
    });
}
