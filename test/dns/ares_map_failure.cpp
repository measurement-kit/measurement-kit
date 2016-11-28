// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/ares_map_failure.hpp"

using namespace mk;

TEST_CASE("map_ares_failure() works as expected") {

    SECTION("For mapped error codes") {
        REQUIRE(dns::map_ares_failure(ARES_SUCCESS).code == NoError().code);
        REQUIRE(dns::map_ares_failure(ARES_EBADNAME).code ==
                dns::BadNameError().code);
        REQUIRE(dns::map_ares_failure(ARES_ENOMEM).code ==
                OutOfMemoryError().code);
    }

    SECTION("For non-mapped error codes") {
        REQUIRE(dns::map_ares_failure(ARES_ETIMEOUT).code ==
                GenericError().code);
    }
}
