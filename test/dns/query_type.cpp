// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

TEST_CASE("QueryType works as expected") {

    auto make_from = [](auto id, auto initial) {
        QueryType value{initial};
        REQUIRE(static_cast<QueryTypeId>(value) == id);
    };

#define XX(_name)                                                              \
    {                                                                          \
        make_from(MK_DNS_TYPE_##_name, #_name);                                \
        make_from(MK_DNS_TYPE_##_name, MK_DNS_TYPE_##_name);                   \
    }
    SECTION("Construction") { MK_DNS_TYPE_IDS }
#undef XX

    auto init_from = [](auto id, auto initial) {
        QueryType value;
        REQUIRE(static_cast<QueryTypeId>(value) == MK_DNS_TYPE_INVALID);
        value = initial;
        REQUIRE(static_cast<QueryTypeId>(value) == id);
    };

#define XX(_name)                                                              \
    {                                                                          \
        init_from(MK_DNS_TYPE_##_name, #_name);                                \
        init_from(MK_DNS_TYPE_##_name, MK_DNS_TYPE_##_name);                   \
    }
    SECTION("Assignment") { MK_DNS_TYPE_IDS }
#undef XX

    SECTION("Comparability to TypeId") {
        QueryType qclass(MK_DNS_TYPE_A);
        REQUIRE(qclass != MK_DNS_TYPE_AAAA);
        REQUIRE(qclass == MK_DNS_TYPE_A);
    }
}
