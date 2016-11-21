// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

TEST_CASE("QueryClass works as expected") {
    QueryClass qclass(QueryClassId::DNS_CLASS_IN);
    QueryClassId id = qclass;
    REQUIRE(id == QueryClassId::DNS_CLASS_IN);
    REQUIRE(qclass != QueryClassId::DNS_CLASS_CH);
    REQUIRE(qclass == QueryClassId::DNS_CLASS_IN);
    REQUIRE(QueryClass("IN") == QueryClassId::DNS_CLASS_IN);
    REQUIRE(QueryClass("CS") == QueryClassId::DNS_CLASS_CS);
    REQUIRE(QueryClass("CH") == QueryClassId::DNS_CLASS_CH);
    REQUIRE(QueryClass("HS") == QueryClassId::DNS_CLASS_HS);
    REQUIRE_THROWS(QueryClass("ANTANI"));
}

TEST_CASE("QueryType works as expected") {
    QueryType qclass(QueryTypeId::DNS_TYPE_A);
    QueryTypeId id = qclass;
    REQUIRE(id == QueryTypeId::DNS_TYPE_A);
    REQUIRE(qclass != QueryTypeId::DNS_TYPE_AAAA);
    REQUIRE(qclass == QueryTypeId::DNS_TYPE_A);
    REQUIRE(QueryType("A") == QueryTypeId::DNS_TYPE_A);
    REQUIRE(QueryType("NS") == QueryTypeId::DNS_TYPE_NS);
    REQUIRE(QueryType("MD") == QueryTypeId::DNS_TYPE_MD);
    REQUIRE(QueryType("MF") == QueryTypeId::DNS_TYPE_MF);
    REQUIRE(QueryType("CNAME") == QueryTypeId::DNS_TYPE_CNAME);
    REQUIRE(QueryType("SOA") == QueryTypeId::DNS_TYPE_SOA);
    REQUIRE(QueryType("MB") == QueryTypeId::DNS_TYPE_MB);
    REQUIRE(QueryType("MG") == QueryTypeId::DNS_TYPE_MG);
    REQUIRE(QueryType("MR") == QueryTypeId::DNS_TYPE_MR);
    REQUIRE(QueryType("NUL") == QueryTypeId::DNS_TYPE_NUL);
    REQUIRE(QueryType("WKS") == QueryTypeId::DNS_TYPE_WKS);
    REQUIRE(QueryType("PTR") == QueryTypeId::DNS_TYPE_PTR);
    REQUIRE(QueryType("HINFO") == QueryTypeId::DNS_TYPE_HINFO);
    REQUIRE(QueryType("MINFO") == QueryTypeId::DNS_TYPE_MINFO);
    REQUIRE(QueryType("MX") == QueryTypeId::DNS_TYPE_MX);
    REQUIRE(QueryType("TXT") == QueryTypeId::DNS_TYPE_TXT);
    REQUIRE(QueryType("AAAA") == QueryTypeId::DNS_TYPE_AAAA);
    REQUIRE(QueryType("REVERSE_A") == QueryTypeId::DNS_TYPE_REVERSE_A);
    REQUIRE(QueryType("REVERSE_AAAA") == QueryTypeId::DNS_TYPE_REVERSE_AAAA);
    REQUIRE_THROWS(QueryType("ANTANI"));
}
