// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/utils.hpp"
#include "private/net/builtin_ca_bundle.hpp"

using namespace mk;

TEST_CASE("The builtin CA bundle is equal to the one in test/fixtures") {
    REQUIRE(*slurpv<uint8_t>("./test/fixtures/saved_ca_bundle.pem") ==
            net::builtin_ca_bundle());
}
