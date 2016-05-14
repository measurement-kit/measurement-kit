// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk::ndt;

TEST_CASE("The NDT test DSL works") {
    NdtTest().set_verbosity(MK_LOG_INFO).run();
}
