/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/net/connection.h's Connection{State,}
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/check_connectivity.hpp>
#include <ight/common/log.hpp>

#include <ight/net/connection.hpp>

using namespace ight::common::check_connectivity;
using namespace ight::common::poller;
using namespace ight::net::connection;
using namespace ight::net::buffer;

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

TEST_CASE("Connection::close() is idempotent") {
    if (Network::is_down()) {
        return;
    }
    Connection s("PF_INET", "nexa.polito.it", "80");
    s.on_connect([&s]() {
        s.enable_read();
        s.send("GET / HTTP/1.0\r\n\r\n");
    });
    s.on_data([&s](SharedPointer<Buffer>) {
        s.close();
        // It shall be safe to call close() more than once
        s.close();
        s.close();
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("It is safe to manipulate Connection after close") {
    if (Network::is_down()) {
        return;
    }
    Connection s("PF_INET", "nexa.polito.it", "80");
    s.on_connect([&s]() {
        s.enable_read();
        s.send("GET / HTTP/1.0\r\n\r\n");
    });
    s.on_data([&s](SharedPointer<Buffer>) {
        s.close();
        // It shall be safe to call any API after close()
	// where safe means that we don't segfault
        REQUIRE_THROWS(s.enable_read());
        REQUIRE_THROWS(s.disable_read());
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("It is safe to close Connection while resolve is in progress") {
    if (Network::is_down()) {
        return;
    }
    ight_set_verbose(1);
    Connection s("PF_INET", "nexa.polito.it", "80");
    DelayedCall unsched(0.001, [&s]() {
        s.close();
    });
    DelayedCall bail_out(2.0, []() {
        ight_break_loop();
    });
    ight_loop();
}
