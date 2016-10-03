// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

// XXX The value of these tests is greatly reduced after class and type
// have become just standard strings

TEST_CASE("QueryClass works as expected") {
    QueryClass qclass("IN");
    REQUIRE(qclass != "CH");
    REQUIRE(qclass == "IN");
    REQUIRE(QueryClass("IN") == "IN");
    REQUIRE(QueryClass("CS") == "CS");
    REQUIRE(QueryClass("CH") == "CH");
    REQUIRE(QueryClass("HS") == "HS");
}

TEST_CASE("QueryType works as expected") {
    QueryType qclass("A");
    REQUIRE(qclass != "AAAA");
    REQUIRE(qclass == "A");
    REQUIRE(QueryType("A") == "A");
    REQUIRE(QueryType("NS") == "NS");
    REQUIRE(QueryType("MD") == "MD");
    REQUIRE(QueryType("MF") == "MF");
    REQUIRE(QueryType("CNAME") == "CNAME");
    REQUIRE(QueryType("SOA") == "SOA");
    REQUIRE(QueryType("MB") == "MB");
    REQUIRE(QueryType("MG") == "MG");
    REQUIRE(QueryType("MR") == "MR");
    REQUIRE(QueryType("NUL") == "NUL");
    REQUIRE(QueryType("WKS") == "WKS");
    REQUIRE(QueryType("PTR") == "PTR");
    REQUIRE(QueryType("HINFO") == "HINFO");
    REQUIRE(QueryType("MINFO") == "MINFO");
    REQUIRE(QueryType("MX") == "MX");
    REQUIRE(QueryType("TXT") == "TXT");
    REQUIRE(QueryType("AAAA") == "AAAA");
    REQUIRE(QueryType("REVERSE_A") == "REVERSE_A");
    REQUIRE(QueryType("REVERSE_AAAA") == "REVERSE_AAAA");
}
