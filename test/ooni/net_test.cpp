// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/ooni/net_test.hpp"

using namespace measurement_kit::ooni;

TEST_CASE("The NetTest should callback when it has finished running") {
    measurement_kit::ooni::NetTest test("");
    test.begin([&]() { test.end([]() { measurement_kit::break_loop(); }); });
    measurement_kit::loop();
}
