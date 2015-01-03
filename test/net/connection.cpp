/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/net/connection.h's IghtConnection{State,}
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "net/connection.h"

TEST_CASE("Ensure that the constructor socket-validity checks work") {

	SECTION("Invalid values are properly normalized") {
		{
			/* Common for both Unix and Windows */
			IghtConnection conn(-1);
			REQUIRE(conn.get_fileno() == -1);
		}
#ifndef WIN32
		{
			IghtConnection conn(-2);
			REQUIRE(conn.get_fileno() == -1);
		}
		/* ... */
		{
			IghtConnection conn(INT_MIN);
			REQUIRE(conn.get_fileno() == -1);
		}
#endif
	}

	SECTION("Valid values are accepted") {
		{
			IghtConnection conn(0);
			REQUIRE(conn.get_fileno() == 0);
		}
		{
			IghtConnection conn(1);
			REQUIRE(conn.get_fileno() == 1);
		}
		{
			IghtConnection conn(2);
			REQUIRE(conn.get_fileno() == 2);
		}
#ifdef WIN32
		{
			IghtConnection conn(INTPTR_MAX);
			REQUIRE(conn.get_fileno() == INTPTR_MAX);
		}
		/* Skip -1 that is INVALID_SOCKET */
		{
			IghtConnection conn(-2);
			REQUIRE(conn.get_fileno() == -2);
		}
		{
			IghtConnection conn(-3);
			REQUIRE(conn.get_fileno() == -3);
		}
		/* ... */
		{
			IghtConnection conn(INTPTR_MIN);
			REQUIRE(conn.get_fileno() == INTPTR_MIN);
		}
#else
		{
			IghtConnection conn(INT_MAX);
			REQUIRE(conn.get_fileno() == INT_MAX);
		}

#endif
	}
}
