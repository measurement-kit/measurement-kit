// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/net/connection.h's Connection{State,}
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

#include <measurement_kit/net.hpp>

#include "src/net/connection.hpp"

using namespace mk;
using namespace mk::net;

TEST_CASE("Ensure that the constructor socket-validity checks work") {
    // TODO: remove
    // Test cancelled because not applicable anymore
}
