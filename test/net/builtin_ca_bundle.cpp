// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/net/builtin_ca_bundle.hpp"

using namespace mk;

TEST_CASE("The builtin CA bundle is equal to the one in test/fixtures") {
    REQUIRE(*slurpv<uint8_t>("./test/fixtures/saved_ca_bundle.pem") ==
            net::builtin_ca_bundle());
}
