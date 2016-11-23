// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common/version.h>

TEST_CASE("The version API works as expected") {
    REQUIRE((mk_version() == std::string{MEASUREMENT_KIT_VERSION}));
}
