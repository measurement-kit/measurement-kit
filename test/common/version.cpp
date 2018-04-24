// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common/version.h>

TEST_CASE("The version API works as expected") {
    REQUIRE((mk_version() == std::string{MEASUREMENT_KIT_VERSION}));
}
