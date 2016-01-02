// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/ooni/ooni_test.hpp"

using namespace mk::ooni;

TEST_CASE("The NetTest should callback when it has finished running") {
    mk::ooni::OoniTest test("");
    test.begin([&]() { test.end([]() { mk::break_loop(); }); });
    mk::loop();
}
