// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"
#include "private/ooni/whatsapp.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

//TEST_CASE("Synchronous whatsapp test") {
//    test::nettests::make_test<WhatsappTest>("meek_fronted_requests.txt")
//        .run();
//}
//
//TEST_CASE("Asynchronous meek-fronted-requests test") {
//    auto t = test::nettests::make_test<WhatsappTest>("meek_fronted_requests.txt");
//    test::nettests::run_async(t);
//}

 TEST_CASE("test net_match") {
     bool no_err, inclusion;
     //no_err = mk::ooni::net_match4("10.10.1.44", "10.10.1.32/27", inclusion);
     //no_err = mk::ooni::net_match4("0.0.0.0", "0.0.0.0/32", inclusion);
     //no_err = mk::ooni::net_match4("0.0.0.1", "0.0.0.1/24", inclusion);
     //no_err = mk::ooni::net_match4("0.0.1.0", "0.0.1.0/24", inclusion);
     //no_err = mk::ooni::net_match4("0.1.0.0", "0.1.0.0/24", inclusion);
     //no_err = mk::ooni::net_match4("1.0.0.0", "1.0.0.0/24", inclusion);
     //no_err = mk::ooni::net_match4("1.0.0.0", "1.0.0.0/24", inclusion);
     //no_err = mk::ooni::net_match4("2000::/3", "2000::/3", inclusion);
 //    REQUIRE( no_err == true );
 //    REQUIRE( inclusion == true );
     //bool b2 = mk::ooni::net_match4("10.10.1.90", "10.10.1.32/27", inclusion);
 //    REQUIRE( no_err == true );
 //    REQUIRE( inclusion == false );
 }

TEST_CASE("ip_to_bytes") {
    std::vector<uint8_t> ex = { 1, 0, 0, 0 };
    std::vector<uint8_t> bs = mk::ooni::ip_to_bytes("1.0.0.0");
    REQUIRE( bs == ex );

    ex = { 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    bs = mk::ooni::ip_to_bytes("ffff::");
    REQUIRE( bs == ex );
}

TEST_CASE("same_pre"){
    std::vector<uint8_t> ip1 = { 1, 0, 0, 0 };
    std::vector<uint8_t> ip2 = { 1, 0, 0, 0 };
    ErrorOr<bool> result = mk::ooni::same_pre(ip1, ip2, 32);
    REQUIRE( !!result );
    REQUIRE( result.as_value() == true );
}

TEST_CASE("same_pre false"){
    std::vector<uint8_t> ip1 = { 1, 0, 0, 0 };
    std::vector<uint8_t> ip2 = { 2, 0, 0, 0 };
    ErrorOr<bool> result = mk::ooni::same_pre(ip1, ip2, 32);
    REQUIRE( !!result );
    REQUIRE( result.as_value() == false );
}

TEST_CASE("same_pre true2"){
    std::vector<uint8_t> ip1 = { 1, 0, 0, 0 };
    std::vector<uint8_t> ip2 = { 1, 0, 0, 255 };
    ErrorOr<bool> result = mk::ooni::same_pre(ip1, ip2, 24);
    REQUIRE( !!result );
    REQUIRE( result.as_value() == true );
}

TEST_CASE("same_pre false2"){
    std::vector<uint8_t> ip1 = { 1, 0, 0, 192 };
    std::vector<uint8_t> ip2 = { 1, 0, 0, 129 };
    ErrorOr<bool> result = mk::ooni::same_pre(ip1, ip2, 26);
    REQUIRE( !!result );
    REQUIRE( result.as_value() == false );
}

TEST_CASE("ip_in_net"){
    ErrorOr<bool> result = mk::ooni::ip_in_net("10.0.0.1", "10.0.0.0/8");
    REQUIRE( !!result );
    REQUIRE( result.as_value() == true );
}

TEST_CASE("ip_in_net false"){
    ErrorOr<bool> result = mk::ooni::ip_in_net("11.0.0.1", "10.0.0.0/8");
    REQUIRE( !!result );
    REQUIRE( result.as_value() == false );
}

#else
int main() {}
#endif
