/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/poller.cpp's Poller()
//

#include "common/poller.h"

#include <iostream>

#ifndef WIN32
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <unistd.h>
#endif

//
// TODO When we use a testing framework, each function describes
// a specific test case.
//

static void
test_event_base_new_failure(void)
{
	auto libevent = IghtLibevent();

	std::cout << "Test event_base_new_failure... ";

	libevent.event_base_new = [](void) {
		return ((event_base *) NULL);
	};

	auto bad_alloc_fired = false;
	try {
		IghtPoller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	if (bad_alloc_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
}

static void
test_evdns_base_new_failure(void)
{
	auto libevent = IghtLibevent();

	std::cout << "Test evdns_base_new_failure... ";

	auto event_base_free_fired = false;

	libevent.event_base_free = [&event_base_free_fired](event_base *b) {
		event_base_free_fired = true;
		::event_base_free(b);
	};
	libevent.evdns_base_new = [](event_base *, int) {
		return ((evdns_base *) NULL);
	};

	auto bad_alloc_fired = false;
	try {
		IghtPoller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	if (bad_alloc_fired && event_base_free_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
}

static void
test_evsignal_failure(void)
{
	auto libevent = IghtLibevent();

	std::cout << "Test evsignal_failure... ";

	auto event_base_free_fired = false;
	auto evdns_base_free_fired = false;

	libevent.event_base_free = [&event_base_free_fired](event_base *b) {
		event_base_free_fired = true;
		::event_base_free(b);
	};
	libevent.evdns_base_free = [&evdns_base_free_fired](evdns_base *b,
	    int opt) {
		evdns_base_free_fired = true;
		::evdns_base_free(b, opt);
	};
	libevent.event_new = [](event_base *, evutil_socket_t, short,
	    event_callback_fn, void *) {
		return ((event *) NULL);
	};

	auto bad_alloc_fired = false;
	try {
		IghtPoller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	if (bad_alloc_fired && event_base_free_fired && evdns_base_free_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
}

static void
test_proper_destruction(void)
{
	auto libevent = IghtLibevent();

	std::cout << "Test proper_destruction... ";

	auto event_base_free_fired = false;
	auto evdns_base_free_fired = false;
	auto event_free_fired = false;

	libevent.event_base_free = [&event_base_free_fired](event_base *b) {
		event_base_free_fired = true;
		::event_base_free(b);
	};
	libevent.evdns_base_free = [&evdns_base_free_fired](evdns_base *b,
	    int opt) {
		evdns_base_free_fired = true;
		::evdns_base_free(b, opt);
	};
	libevent.event_free = [&event_free_fired](event *e) {
		event_free_fired = true;
		::event_free(e);
	};

	{
		IghtPoller poller(&libevent);
	}

	if (event_base_free_fired && evdns_base_free_fired && event_free_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
}

static void
test_event_add_failure(void)
{
#ifndef WIN32
	auto libevent = IghtLibevent();

	std::cout << "Test event_add_failure... ";

	libevent.event_add = [](event *, timeval *) {
		return (-1);
	};

	IghtPoller poller(&libevent);

	auto runtime_error_fired = false;
	try {
		poller.break_loop_on_sigint_(true);
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	if (runtime_error_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
#endif
}

static void
test_event_del_failure(void)
{
#ifndef WIN32
	auto libevent = IghtLibevent();

	std::cout << "Test event_del_failure... ";

	libevent.event_del = [](event *) {
		return (-1);
	};

	IghtPoller poller(&libevent);
	poller.break_loop_on_sigint_(true);

	auto runtime_error_fired = false;
	try {
		poller.break_loop_on_sigint_(false);
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	if (runtime_error_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
#endif
}

static void
test_multiple_break_loop_on_sigint_(void)
{
#ifndef WIN32
	std::cout << "Test multiple_break_loop_on_sigint_... ";

	//
	// Make sure it's OK (as it ought to be) to add() or del()
	// multiple times the evsignal event.
	//

	{
		IghtPoller poller;
		poller.break_loop_on_sigint_(false);
		poller.break_loop_on_sigint_(false);
		std::cout << "ok ";
	}

	{
		IghtPoller poller;
		poller.break_loop_on_sigint_(true);
		poller.break_loop_on_sigint_(true);
		std::cout << "ok ";
	}

	{
		IghtPoller poller;
		poller.break_loop_on_sigint_(true);
		poller.break_loop_on_sigint_(false);
		poller.break_loop_on_sigint_(false);
		std::cout << "ok ";
	}

	std::cout << std::endl;
#endif
}

static void
test_loop(void)
{
	IghtLibevent libevent;

	std::cout << "Test loop... ";

	//
	// Scenario #1
	//

	libevent.event_base_dispatch = [](event_base *) {
		return (-1);
	};

	IghtPoller poller1(&libevent);

	auto runtime_error_fired = false;
	try {
		poller1.loop();
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	if (runtime_error_fired)
		std::cout << "ok ";
	else
		std::cout << "FAIL ";

	//
	// Scenario #2
	//

	libevent.event_base_dispatch = [](event_base *) {
		return (1);
	};
	IghtPoller poller2(&libevent);
	poller2.loop();
	std::cout << "ok ";

	std::cout << std::endl;
}

static void
test_break_loop(void)
{
	IghtLibevent libevent;

	std::cout << "Test break_loop... ";

	libevent.event_base_loopbreak = [](event_base *) {
		return (-1);
	};

	IghtPoller poller(&libevent);

	auto runtime_error_fired = false;
	try {
		poller.break_loop();
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	if (runtime_error_fired)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
}

static void
test_sigint_correctly_handled(void)
{
#ifndef WIN32
	std::cout << "Test sigint_correctly_handled... ";

	IghtPoller poller;
	auto d = IghtDelayedCall(0.01, [](void) {
	    raise(SIGINT);
	}, NULL, poller.get_event_base());
	poller.break_loop_on_sigint_();
	poller.loop();

	std::cout << "ok" << std::endl;
#endif
}

static void
test_sigint_correctly_handled_once_removed(void)
{
#ifndef WIN32
	std::cout << "Test sigint_correctly_handled_once_removed... ";

	int status;
	pid_t pid;

	if ((pid = fork()) < 0)
		throw std::runtime_error("fork failed");

	if (pid == 0) {
		IghtPoller poller;
		auto d = IghtDelayedCall(0.01, [](void) {
			raise(SIGINT);
		}, NULL, poller.get_event_base());
		/*
		 * Note: I want to make sure that the behavior is as expected
		 * after the signal handler is removed.
		 */
		poller.break_loop_on_sigint_();
		poller.break_loop_on_sigint_(false);
		poller.loop();
		exit(EXIT_FAILURE);	/* Should not happen */
	}

	if (waitpid(pid, &status, 0) < 0)
		throw std::runtime_error("waitpid failed");

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		std::cout << "ok";
	else
		std::cout << "FAIL";

	std::cout << std::endl;
#endif
}

int
main(void)
{
	test_event_base_new_failure();
	test_evdns_base_new_failure();
	test_evsignal_failure();
	test_proper_destruction();
	test_event_add_failure();
	test_event_del_failure();
	test_multiple_break_loop_on_sigint_();
	test_loop();
	test_break_loop();
	test_sigint_correctly_handled();
	test_sigint_correctly_handled_once_removed();
}
