// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common/version.hpp>

TEST_CASE("The version API works as expected") {
    REQUIRE((mk::library_version() == MEASUREMENT_KIT_VERSION));
}
