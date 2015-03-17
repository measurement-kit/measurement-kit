/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/poller.cpp's Poller()
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/poller.hpp>

#ifndef WIN32
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <unistd.h>
#endif

using namespace ight::common::libevent;
using namespace ight::common::poller;

TEST_CASE("Constructor") {

    SECTION("We deal with event_base_new() failure") {
	auto libevent = Libevent();

	libevent.event_base_new = [](void) {
		return ((event_base *) NULL);
	};

	auto bad_alloc_fired = false;
	try {
		Poller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	REQUIRE(bad_alloc_fired);
    }

    SECTION("We deal with evdns_base_new() failure") {
	auto libevent = Libevent();

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
		Poller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	REQUIRE(bad_alloc_fired);
	REQUIRE(event_base_free_fired);
    }

    SECTION("We deal with evsignal failure") {
	auto libevent = Libevent();

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
		Poller poller(&libevent);
	} catch (std::bad_alloc&) {
		bad_alloc_fired = true;
	}

	REQUIRE(bad_alloc_fired);
	REQUIRE(event_base_free_fired);
	REQUIRE(evdns_base_free_fired);
    }
}

TEST_CASE("The destructor works properly") {

	auto libevent = Libevent();

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
		Poller poller(&libevent);
	}

	REQUIRE(event_base_free_fired);
	REQUIRE(evdns_base_free_fired);
	REQUIRE(event_free_fired);
}

TEST_CASE("We deal with event_add() failure in break_loop_on_sigint_()") {
#ifndef WIN32
	auto libevent = Libevent();

	libevent.event_add = [](event *, timeval *) {
		return (-1);
	};

	Poller poller(&libevent);

	auto runtime_error_fired = false;
	try {
		poller.break_loop_on_sigint_(true);
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	REQUIRE(runtime_error_fired);
#endif
}

TEST_CASE("We deal with event_del() failure in break_loop_on_sigint_()") {
#ifndef WIN32
	auto libevent = Libevent();

	libevent.event_del = [](event *) {
		return (-1);
	};

	Poller poller(&libevent);
	poller.break_loop_on_sigint_(true);

	auto runtime_error_fired = false;
	try {
		poller.break_loop_on_sigint_(false);
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	REQUIRE(runtime_error_fired);
#endif
}

TEST_CASE("break_loop_on_sigint_() is idempotent") {
#ifndef WIN32

	//
	// Make sure it's OK (as it ought to be) to add() or del()
	// multiple times the evsignal event.
	//

	{
		Poller poller;
		poller.break_loop_on_sigint_(false);
		poller.break_loop_on_sigint_(false);
	}

	{
		Poller poller;
		poller.break_loop_on_sigint_(true);
		poller.break_loop_on_sigint_(true);
	}

	{
		Poller poller;
		poller.break_loop_on_sigint_(true);
		poller.break_loop_on_sigint_(false);
		poller.break_loop_on_sigint_(false);
	}
#endif
}

TEST_CASE("poller.loop() works properly in corner cases") {


    SECTION("We deal with event_base_dispatch() returning -1") {
	Libevent libevent;

	libevent.event_base_dispatch = [](event_base *) {
		return (-1);
	};

	Poller poller1(&libevent);

	auto runtime_error_fired = false;
	try {
		poller1.loop();
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	REQUIRE(runtime_error_fired);
    }

    SECTION("We deal with event_base_dispatch() returning 1") {
	Libevent libevent;

	libevent.event_base_dispatch = [](event_base *) {
		return (1);
	};
	Poller poller2(&libevent);
	poller2.loop();
    }
}

TEST_CASE("poller.break_loop() works properly") {
	Libevent libevent;

	libevent.event_base_loopbreak = [](event_base *) {
		return (-1);
	};

	Poller poller(&libevent);

	auto runtime_error_fired = false;
	try {
		poller.break_loop();
	} catch (std::runtime_error&) {
		runtime_error_fired = true;
	}

	REQUIRE(runtime_error_fired);
}

TEST_CASE("SIGINT is correctly handled on Unix") {
#ifndef WIN32

    SECTION("SIGINT is correctly handled after the handler is set") {
	Poller poller;

	DelayedCall d(0.01, [](void) {
	    raise(SIGINT);
	}, NULL, poller.get_event_base());

	poller.break_loop_on_sigint_();
	poller.loop();
    }

    SECTION("SIGINT is correctly handled after the handler is removed") {

	int status;
	pid_t pid;

	pid = fork();
	REQUIRE(pid >= 0);

	if (pid == 0) {
		Poller poller;
		DelayedCall d(0.01, [](void) {
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

	pid = waitpid(pid, &status, 0);
	REQUIRE(pid >= 0);

	REQUIRE(WIFSIGNALED(status));
	REQUIRE(WTERMSIG(status) == SIGINT);
    }
#endif
}
