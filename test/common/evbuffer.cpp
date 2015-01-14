/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's IghtEvbuffer
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "common/libevent.h"

TEST_CASE("The constructor is lazy") {

	auto libevent = IghtLibevent();
	auto calls = 0;

	libevent.evbuffer_new = [&](void) {
		++calls;
		return ((evbuffer *) NULL);
	};
	libevent.evbuffer_free = [&](evbuffer *) {
		++calls;
	};

	{
		IghtEvbuffer evbuf(&libevent);
	}

	REQUIRE(calls == 0);
}

TEST_CASE("The (evbuffer*) operation allocates the internal evbuffer") {

	auto libevent = IghtLibevent();
	auto calls = 0;

	libevent.evbuffer_new = [&](void) {
		++calls;
		return (::evbuffer_new());
	};
	libevent.evbuffer_free = [&](evbuffer *e) {
		++calls;
		evbuffer_free(e);
	};

	{
		IghtEvbuffer evbuf(&libevent);
		auto p = (evbuffer *) evbuf;
		(void) p;
	}

	REQUIRE(calls == 2);
}

TEST_CASE("The (evbuffer *) operation is idempotent") {

	auto libevent = IghtLibevent();
	auto calls = 0;

	libevent.evbuffer_new = [&](void) {
		++calls;
		return (::evbuffer_new());
	};

	IghtEvbuffer evbuf(&libevent);
	auto p1 = (evbuffer *) evbuf;
	auto p2 = (evbuffer *) evbuf;

	REQUIRE(p1 == p2);
	REQUIRE(calls == 1);
}

TEST_CASE("std::bad_alloc is raised when out of memory") {

	auto libevent = IghtLibevent();

	libevent.evbuffer_new = [&](void) {
		return ((evbuffer *) NULL);
	};

	REQUIRE_THROWS_AS([&](void) {
		/* Yes, I really really love inline functions <3 */
		IghtEvbuffer evbuf(&libevent);
		auto p = (evbuffer *) evbuf;
		(void) p;
	}(), std::bad_alloc);
}
