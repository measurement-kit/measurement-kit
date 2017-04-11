// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/common/utils_impl.hpp"
#include "../src/libmeasurement_kit/net/builtin_ca_bundle.hpp"

using namespace mk;

TEST_CASE("The builtin CA bundle is equal to the one in test/fixtures") {
    REQUIRE(*slurpv_impl<uint8_t>("./test/fixtures/saved_ca_bundle.pem") ==
            net::builtin_ca_bundle());
}
