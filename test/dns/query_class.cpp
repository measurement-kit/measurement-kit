// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

TEST_CASE("QueryClass works as expected") {

    auto make_from = [](auto id, auto initial) {
        QueryClass value{initial};
        REQUIRE(static_cast<QueryClassId>(value) == id);
    };

#define XX(_name)                                                              \
    {                                                                          \
        make_from(MK_DNS_CLASS_##_name, #_name);                               \
        make_from(MK_DNS_CLASS_##_name, MK_DNS_CLASS_##_name);                 \
    }
    SECTION("Construction") { MK_DNS_CLASS_IDS }
#undef XX

    auto init_from = [](auto id, auto initial) {
        QueryClass value;
        REQUIRE(static_cast<QueryClassId>(value) == MK_DNS_CLASS_INVALID);
        value = initial;
        REQUIRE(static_cast<QueryClassId>(value) == id);
    };

#define XX(_name)                                                              \
    {                                                                          \
        init_from(MK_DNS_CLASS_##_name, #_name);                               \
        init_from(MK_DNS_CLASS_##_name, MK_DNS_CLASS_##_name);                 \
    }
    SECTION("Assignment") { MK_DNS_CLASS_IDS }
#undef XX

    SECTION("Comparability to ClassId") {
        QueryClass qclass(MK_DNS_CLASS_IN);
        REQUIRE(qclass != MK_DNS_CLASS_CH);
        REQUIRE(qclass == MK_DNS_CLASS_IN);
    }
}
