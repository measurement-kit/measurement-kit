// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

TEST_CASE("QueryClass works as expected") {
    QueryClass qclass(MK_DNS_CLASS_IN);
    QueryClassId id = qclass;
    REQUIRE(id == MK_DNS_CLASS_IN);
    REQUIRE(qclass != MK_DNS_CLASS_CH);
    REQUIRE(qclass == MK_DNS_CLASS_IN);
    REQUIRE(QueryClass("IN") == MK_DNS_CLASS_IN);
    REQUIRE(QueryClass("CS") == MK_DNS_CLASS_CS);
    REQUIRE(QueryClass("CH") == MK_DNS_CLASS_CH);
    REQUIRE(QueryClass("HS") == MK_DNS_CLASS_HS);
    REQUIRE(QueryClass("ANTANI") == MK_DNS_CLASS_INVALID);
}

TEST_CASE("QueryType works as expected") {
    QueryType qclass(MK_DNS_TYPE_A);
    QueryTypeId id = qclass;
    REQUIRE(id == MK_DNS_TYPE_A);
    REQUIRE(qclass != MK_DNS_TYPE_AAAA);
    REQUIRE(qclass == MK_DNS_TYPE_A);
    REQUIRE(QueryType("A") == MK_DNS_TYPE_A);
    REQUIRE(QueryType("NS") == MK_DNS_TYPE_NS);
    REQUIRE(QueryType("MD") == MK_DNS_TYPE_MD);
    REQUIRE(QueryType("MF") == MK_DNS_TYPE_MF);
    REQUIRE(QueryType("CNAME") == MK_DNS_TYPE_CNAME);
    REQUIRE(QueryType("SOA") == MK_DNS_TYPE_SOA);
    REQUIRE(QueryType("MB") == MK_DNS_TYPE_MB);
    REQUIRE(QueryType("MG") == MK_DNS_TYPE_MG);
    REQUIRE(QueryType("MR") == MK_DNS_TYPE_MR);
    REQUIRE(QueryType("NUL") == MK_DNS_TYPE_NUL);
    REQUIRE(QueryType("WKS") == MK_DNS_TYPE_WKS);
    REQUIRE(QueryType("PTR") == MK_DNS_TYPE_PTR);
    REQUIRE(QueryType("HINFO") == MK_DNS_TYPE_HINFO);
    REQUIRE(QueryType("MINFO") == MK_DNS_TYPE_MINFO);
    REQUIRE(QueryType("MX") == MK_DNS_TYPE_MX);
    REQUIRE(QueryType("TXT") == MK_DNS_TYPE_TXT);
    REQUIRE(QueryType("AAAA") == MK_DNS_TYPE_AAAA);
    REQUIRE(QueryType("REVERSE_A") == MK_DNS_TYPE_REVERSE_A);
    REQUIRE(QueryType("REVERSE_AAAA") == MK_DNS_TYPE_REVERSE_AAAA);
    REQUIRE(QueryType("ANTANI") == MK_DNS_TYPE_INVALID);
}
