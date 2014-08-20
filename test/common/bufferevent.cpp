/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's IghtBufferevent
//

#include "common/poller.h"

#include <iostream>

static void
test_empty_constructor(void)
{
	auto libevent = IghtLibevent();
	auto calls = 0;

	std::cout << "test empty_constructor... ";

	libevent.bufferevent_socket_new = [&](event_base *,
	    evutil_socket_t, int) {
		++calls;
		return ((bufferevent *) NULL);
	};
	libevent.bufferevent_free = [&](bufferevent *) {
		++calls;
	};

	{
		auto b = IghtBuffereventSocket(&libevent);
	}

	if (calls != 0)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_raise_if_bev_is_null(void)
{
	auto raised_runtime_error = false;

	std::cout << "test raise_if_bev_is_null... ";

	try {
		auto b = IghtBuffereventSocket();
		auto p = (bufferevent *) b;
		(void) p;
	} catch (std::runtime_error&) {
		raised_runtime_error = true;
	}

	if (!raised_runtime_error)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_real_constructor(void)
{
	auto libevent = IghtLibevent();
	auto calls = 0;

	std::cout << "test real_constructor... ";

	libevent.bufferevent_socket_new = [&](
	    event_base *b, evutil_socket_t s, int o) {
		++calls;
		return (::bufferevent_socket_new(b, s, o));
	};
	libevent.bufferevent_free = [&](bufferevent *b) {
		++calls;
		::bufferevent_free(b);
	};

	{
		auto poller = IghtGlobalPoller::get();
		auto b = IghtBuffereventSocket(poller->get_event_base(),
		    -1, 0, &libevent);
	}

	if (calls != 2)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_oom(void)
{
	auto libevent = IghtLibevent();
	auto raised = 0;

	std::cout << "test oom... ";

	libevent.bufferevent_socket_new = [&](event_base *,
	    evutil_socket_t, int) {
		return ((bufferevent *) NULL);
	};

	try {
		auto base = IghtGlobalPoller::get()->get_event_base();
		auto bev = IghtBuffereventSocket(base, 0, 0, &libevent);
	} catch (std::bad_alloc&) {
		++raised;
	}

	if (!raised)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_access_underlying_bev_ok(void)
{
	auto libevent = IghtLibevent();
	auto underlying = (bufferevent *) NULL;

	std::cout << "test access_underlying_bev_ok... ";

	libevent.bufferevent_socket_new = [&](event_base *b,
	    evutil_socket_t s, int o) {
		return (underlying = ::bufferevent_socket_new(b, s, o));
	};

	auto poller = IghtGlobalPoller::get();
	auto b = IghtBuffereventSocket(poller->get_event_base(), -1, 0,
	    &libevent);

	if (underlying != (bufferevent *) b)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_move_constructor(void)
{
	std::cout << "test move_constructor... ";

	auto libevent = IghtLibevent();
	auto underlying = (bufferevent *) NULL;

	libevent.bufferevent_socket_new = [&](event_base *b,
	    evutil_socket_t s, int o) {
		return (underlying = ::bufferevent_socket_new(b, s, o));
	};

	auto poller = IghtGlobalPoller::get();
	auto b1 = IghtBuffereventSocket(poller->get_event_base(),
	    0, 0, &libevent);
	auto b2 = IghtBuffereventSocket(std::move(b1));

	if (b2.get_libevent() != &libevent)
		throw std::runtime_error("FAIL");
	if (underlying != (bufferevent *) b2)
		throw std::runtime_error("FAIL");
	if (b1.get_libevent() != IghtGlobalLibevent::get())
		throw std::runtime_error("FAIL");

	auto raised = 0;
	try {
		auto ppp = (bufferevent *) b1;
		(void) ppp;
	} catch (std::runtime_error&) {
		++raised;
	}
	if (!raised)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

static void
test_move_assignment(void)
{
	std::cout << "test move_assignment... ";

	auto libevent1 = IghtLibevent();
	auto underlying = (bufferevent *) NULL;

	libevent1.bufferevent_socket_new = [&](event_base *b,
	    evutil_socket_t s, int o) {
		return (underlying = ::bufferevent_socket_new(b, s, o));
	};

	auto poller = IghtGlobalPoller::get();
	auto b1 = IghtBuffereventSocket(poller->get_event_base(),
	    0, 0, &libevent1);
	auto libevent2 = IghtLibevent();
	auto b2 = IghtBuffereventSocket(&libevent2);
	b2 = std::move(b1);

	if (b2.get_libevent() != &libevent1)
		throw std::runtime_error("FAIL");
	if (underlying != (bufferevent *) b2)
		throw std::runtime_error("FAIL");
	if (b1.get_libevent() != &libevent2)
		throw std::runtime_error("FAIL");

	auto raised = 0;
	try {
		auto ppp = (bufferevent *) b1;
		(void) ppp;
	} catch (std::runtime_error&) {
		++raised;
	}
	if (!raised)
		throw std::runtime_error("FAIL");

	std::cout << "ok" << std::endl;
}

int
main(void)
{
	test_empty_constructor();
	test_raise_if_bev_is_null();
	test_real_constructor();
	test_oom();
	test_access_underlying_bev_ok();
	test_move_constructor();
	test_move_assignment();
}
