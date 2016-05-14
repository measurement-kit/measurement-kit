// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

TEST_CASE("QueryClass works as expected") {
    QueryClass qclass(QueryClassId::IN);
    QueryClassId id = qclass;
    REQUIRE(id == QueryClassId::IN);
    REQUIRE(qclass != QueryClassId::CH);
    REQUIRE(qclass == QueryClassId::IN);
    REQUIRE(QueryClass("IN") == QueryClassId::IN);
    REQUIRE(QueryClass("CS") == QueryClassId::CS);
    REQUIRE(QueryClass("CH") == QueryClassId::CH);
    REQUIRE(QueryClass("HS") == QueryClassId::HS);
    REQUIRE_THROWS(QueryClass("ANTANI"));
}

TEST_CASE("QueryType works as expected") {
    QueryType qclass(QueryTypeId::A);
    QueryTypeId id = qclass;
    REQUIRE(id == QueryTypeId::A);
    REQUIRE(qclass != QueryTypeId::AAAA);
    REQUIRE(qclass == QueryTypeId::A);
    REQUIRE(QueryType("A") == QueryTypeId::A);
    REQUIRE(QueryType("NS") == QueryTypeId::NS);
    REQUIRE(QueryType("MD") == QueryTypeId::MD);
    REQUIRE(QueryType("MF") == QueryTypeId::MF);
    REQUIRE(QueryType("CNAME") == QueryTypeId::CNAME);
    REQUIRE(QueryType("SOA") == QueryTypeId::SOA);
    REQUIRE(QueryType("MB") == QueryTypeId::MB);
    REQUIRE(QueryType("MG") == QueryTypeId::MG);
    REQUIRE(QueryType("MR") == QueryTypeId::MR);
    REQUIRE(QueryType("NUL") == QueryTypeId::NUL);
    REQUIRE(QueryType("WKS") == QueryTypeId::WKS);
    REQUIRE(QueryType("PTR") == QueryTypeId::PTR);
    REQUIRE(QueryType("HINFO") == QueryTypeId::HINFO);
    REQUIRE(QueryType("MINFO") == QueryTypeId::MINFO);
    REQUIRE(QueryType("MX") == QueryTypeId::MX);
    REQUIRE(QueryType("TXT") == QueryTypeId::TXT);
    REQUIRE(QueryType("AAAA") == QueryTypeId::AAAA);
    REQUIRE(QueryType("REVERSE_A") == QueryTypeId::REVERSE_A);
    REQUIRE(QueryType("REVERSE_AAAA") == QueryTypeId::REVERSE_AAAA);
    REQUIRE_THROWS(QueryType("ANTANI"));
}
