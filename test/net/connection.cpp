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
			auto conn = IghtConnection(-1);
			REQUIRE(conn.get_fileno() == -1);
		}
#ifndef WIN32
		{
			auto conn = IghtConnection(-2);
			REQUIRE(conn.get_fileno() == -1);
		}
		/* ... */
		{
			auto conn = IghtConnection(INT_MIN);
			REQUIRE(conn.get_fileno() == -1);
		}
#endif
	}

	SECTION("Valid values are accepted") {
		{
			auto conn = IghtConnection(0);
			REQUIRE(conn.get_fileno() == 0);
		}
		{
			auto conn = IghtConnection(1);
			REQUIRE(conn.get_fileno() == 1);
		}
		{
			auto conn = IghtConnection(2);
			REQUIRE(conn.get_fileno() == 2);
		}
#ifdef WIN32
		{
			auto conn = IghtConnection(INTPTR_MAX);
			REQUIRE(conn.get_fileno() == INTPTR_MAX);
		}
		/* Skip -1 that is INVALID_SOCKET */
		{
			auto conn = IghtConnection(-2);
			REQUIRE(conn.get_fileno() == -2);
		}
		{
			auto conn = IghtConnection(-3);
			REQUIRE(conn.get_fileno() == -3);
		}
		/* ... */
		{
			auto conn = IghtConnection(INTPTR_MIN);
			REQUIRE(conn.get_fileno() == INTPTR_MIN);
		}
#else
		{
			auto conn = IghtConnection(INT_MAX);
			REQUIRE(conn.get_fileno() == INT_MAX);
		}

#endif
	}
}
