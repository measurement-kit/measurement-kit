// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/net/connection.h's Connection{State,}
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

#include <measurement_kit/net.hpp>

#include "src/net/connection.hpp"

using namespace mk;
using namespace mk::net;

TEST_CASE("Ensure that the constructor socket-validity checks work") {

    SECTION("Invalid values are properly normalized") {
        {
            /* Common for both Unix and Windows */
            Connection conn(-1);
            REQUIRE(conn.get_fileno() == -1);
        }
#ifndef WIN32
        {
            Connection conn(-2);
            REQUIRE(conn.get_fileno() == -1);
        }
        /* ... */
        {
            Connection conn(INT_MIN);
            REQUIRE(conn.get_fileno() == -1);
        }
#endif
    }

    SECTION("Valid values are accepted") {
        {
            Connection conn(0);
            REQUIRE(conn.get_fileno() == 0);
        }
        {
            Connection conn(1);
            REQUIRE(conn.get_fileno() == 1);
        }
        {
            Connection conn(2);
            REQUIRE(conn.get_fileno() == 2);
        }
#ifdef WIN32
        {
            Connection conn(INTPTR_MAX);
            REQUIRE(conn.get_fileno() == INTPTR_MAX);
        }
        /* Skip -1 that is INVALID_SOCKET */
        {
            Connection conn(-2);
            REQUIRE(conn.get_fileno() == -2);
        }
        {
            Connection conn(-3);
            REQUIRE(conn.get_fileno() == -3);
        }
        /* ... */
        {
            Connection conn(INTPTR_MIN);
            REQUIRE(conn.get_fileno() == INTPTR_MIN);
        }
#else
        {
            Connection conn(INT_MAX);
            REQUIRE(conn.get_fileno() == INT_MAX);
        }

#endif
    }
}
