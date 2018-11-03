// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/net/builtin_ca_bundle.h"

using namespace mk;

TEST_CASE("The builtin CA bundle is equal to the one in test/fixtures") {
    std::basic_string<uint8_t> s{mk_ca_bundle_pem,
          (size_t)mk_ca_bundle_pem_len};
    std::vector<uint8_t> v{s.begin(), s.end()};
    REQUIRE(*slurpv<uint8_t>("./test/fixtures/saved_ca_bundle.pem") == v);
}
