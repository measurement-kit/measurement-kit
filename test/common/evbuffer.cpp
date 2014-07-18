/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's IghtEvbuffer
//

#include "common/libevent.h"

#include <iostream>

static void
test_lazy_construct(void)
{
	auto libevent = IghtLibevent();
	auto calls = 0;

	std::cout << "test lazy_construct... ";

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

	if (calls != 0)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_alloc_if_cast(void)
{
	auto libevent = IghtLibevent();
	auto calls = 0;

	std::cout << "test alloc_if_cast... ";

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

	if (calls != 2)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_multiple_cast_ok(void)
{
	std::cout << "test multiple_cast_ok... ";

	auto libevent = IghtLibevent();
	auto calls = 0;

	libevent.evbuffer_new = [&](void) {
		++calls;
		return (::evbuffer_new());
	};

	auto evbuf = IghtEvbuffer(&libevent);
	auto p1 = (evbuffer *) evbuf;
	auto p2 = (evbuffer *) evbuf;

	if (p1 != p2)
		throw std::runtime_error("FAIL");
	if (calls != 1)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_oom(void)
{
	auto libevent = IghtLibevent();
	auto raised = 0;

	std::cout << "test oom... ";

	libevent.evbuffer_new = [&](void) {
		return ((evbuffer *) NULL);
	};

	try {
		auto evbuf = IghtEvbuffer(&libevent);
		auto p = (evbuffer *) evbuf;
		(void) p;
	} catch (std::bad_alloc&) {
		++raised;
	}

	if (!raised)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_move_constructor(void)
{
	auto libevent = IghtLibevent();

	auto underlying = (evbuffer *) NULL;
	libevent.evbuffer_new = [&](void) {
		return (underlying = ::evbuffer_new());
	};

	std::cout << "test move_constructor... ";

	auto evbuf1 = IghtEvbuffer(&libevent);
	auto p1 = (evbuffer *) evbuf1;
	(void) p1;

	auto evbuf2 = std::move(evbuf1);

	if ((evbuffer *) evbuf2 != underlying)
		throw std::runtime_error("FAIL");
	if (evbuf2.get_libevent() != &libevent)
		throw std::runtime_error("FAIL");
	if ((evbuffer *) evbuf1 == underlying)
		throw std::runtime_error("FAIL");
	if (evbuf1.get_libevent() != IghtGlobalLibevent::get())
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_move_assignment(void)
{
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

	std::cout << "test move_assignment... ";

	auto evbuf1 = IghtEvbuffer(&libevent1);
	auto p1 = (evbuffer *) evbuf1;
	(void) p1;

	auto evbuf2 = IghtEvbuffer(&libevent2);
	auto p2 = (evbuffer *) evbuf2;
	(void) p2;

	evbuf2 = std::move(evbuf1);

	if ((evbuffer *) evbuf2 != underlying)
		throw std::runtime_error("FAIL");
	if (evbuf2.get_libevent() != &libevent1)
		throw std::runtime_error("FAIL");
	if ((evbuffer *) evbuf1 != (evbuffer *) 1234)
		throw std::runtime_error("FAIL");
	if (evbuf1.get_libevent() != &libevent2)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

int
main(void)
{
	test_lazy_construct();
	test_alloc_if_cast();
	test_multiple_cast_ok();
	test_oom();
	test_move_constructor();
	test_move_assignment();
}
