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
		auto evbuf = IghtEvbuffer(&libevent);
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
		auto evbuf = IghtEvbuffer(&libevent);
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

	auto evbuf = IghtEvbuffer(&libevent);
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
		auto evbuf = IghtEvbuffer(&libevent);
		auto p = (evbuffer *) evbuf;
		(void) p;
	}(), std::bad_alloc);
}

TEST_CASE("Move semantic") {

    SECTION("Move constructor") {
	auto libevent = IghtLibevent();

	auto underlying = (evbuffer *) NULL;
	libevent.evbuffer_new = [&](void) {
		return (underlying = ::evbuffer_new());
	};

	auto evbuf1 = IghtEvbuffer(&libevent);
	auto p1 = (evbuffer *) evbuf1;
	(void) p1;

	auto evbuf2 = std::move(evbuf1);

	REQUIRE((evbuffer *) evbuf2 == underlying);
	REQUIRE(evbuf2.get_libevent() == &libevent);
	REQUIRE((evbuffer *) evbuf1 != underlying);
	REQUIRE(evbuf1.get_libevent() == IghtGlobalLibevent::get());
    }

    SECTION("Move assignment") {

	auto libevent1 = IghtLibevent(),
	    libevent2 = IghtLibevent();

	auto underlying = (evbuffer *)NULL;
	libevent1.evbuffer_new = [&](void) {
		return (underlying = ::evbuffer_new());
	};
	libevent2.evbuffer_new = [&](void) {
		return ((evbuffer *) 1234);
	};
	libevent2.evbuffer_free = [&](evbuffer *) {
		/* nothing!!! */
	};

	auto evbuf1 = IghtEvbuffer(&libevent1);
	auto p1 = (evbuffer *) evbuf1;
	(void) p1;

	auto evbuf2 = IghtEvbuffer(&libevent2);
	auto p2 = (evbuffer *) evbuf2;
	(void) p2;

	evbuf2 = std::move(evbuf1);

	REQUIRE((evbuffer *) evbuf2 == underlying);
	REQUIRE(evbuf2.get_libevent() == &libevent1);
	REQUIRE((evbuffer *) evbuf1 == (evbuffer *) 1234);
	REQUIRE (evbuf1.get_libevent() == &libevent2);
    }
}
