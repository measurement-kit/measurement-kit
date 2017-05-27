// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>
#include "../src/libmeasurement_kit/common/utils.hpp"
#include "../src/libmeasurement_kit/libevent/poller_impl.hpp"

using namespace mk;
using namespace mk::libevent;

static int fail_int() { return -1; }
static event_base *fail_evbase() { return nullptr; }

static int fail(int, const struct sigaction *, struct sigaction *) {
    return -1;
}

static bool event_base_free_called = false;
static void event_base_free_mock(event_base *p) {
    REQUIRE(!event_base_free_called);
    event_base_free(p);
    event_base_free_called = true;
}

TEST_CASE("Constructor") {
    SECTION("We deal with evthread_use_pthreads() failure") {
        // Since here we syntethize a different template, this should be
        // the time where we create the singleton()
        REQUIRE_THROWS(poller_alloc_evbase<fail_int>());
    }

    SECTION("We deal with sigaction() failure") {
        REQUIRE_THROWS((poller_alloc_evbase<evthread_use_pthreads, fail>()));
    }

    SECTION("We deal with event_base_new() failure") {
        REQUIRE_THROWS((poller_alloc_evbase<evthread_use_pthreads, sigaction,
                                            fail_evbase>()));
    }
}

TEST_CASE("The destructor works properly") {
    {
        poller_alloc_evbase<evthread_use_pthreads, sigaction, event_base_new,
                            event_base_free_mock>();
    }
    REQUIRE(event_base_free_called);
}

static int fail(event_base *, evutil_socket_t, short, event_callback_fn,
                void *, const timeval *) {
    return -1;
}

TEST_CASE("call_later() deals with event_base_once() failure") {
    REQUIRE_THROWS((poller_call_later<fail>(
            poller_alloc_evbase(), 1.0, [](){})));
}

static event *fail(struct event_base *, evutil_socket_t, short,
                   event_callback_fn, void *) {
    return nullptr;
}

static bool event_free_called = false;
static void event_free_mock(event *p) {
    REQUIRE(!event_free_called);
    event_free(p);
    event_free_called = true;
}

static int fail(event *, const timeval *) {
    return -1;
}

static int fail(event_base *) {
    return -1;
}

static int returns_one(event_base *) {
    return 1;
}

TEST_CASE("poller.loop() works properly in corner cases") {

    SECTION("We deal with event_new() failure") {
        Poller poller;
        REQUIRE_THROWS(poller_loop<fail>(poller.base_, &poller, false));
    }

    SECTION("We free the periodic event") {
        Poller poller;
        poller.call_later(1.0, [&poller]() { poller.break_loop(); });
        poller_loop<event_new, event_free_mock>(poller.base_, &poller, false);
        REQUIRE(event_free_called);
    }

    SECTION("We deal with event_add() failure") {
        Poller poller;
        REQUIRE_THROWS((poller_loop<event_new, event_free, fail>(
                poller.base_, &poller, false)));
    }

    SECTION("We deal with event_base_dispatch() returning -1") {
        Poller poller;
        REQUIRE_THROWS((poller_loop<event_new, event_free, event_add, fail>(
                poller.base_, &poller, false)));
    }

    SECTION("We do not throw when event_base_dispatch() returs 1") {
        Poller poller;
        poller_loop<event_new, event_free, event_add, returns_one>(
                poller.base_, &poller, false);
    }

    SECTION("The autostop flag is honoured") {
        Poller poller;
        poller.set_autostop(true);
        REQUIRE(poller_loop(poller.base_, &poller, true) == 1);
    }
}

static int fail(event_base *, int) {
    return -1;
}

TEST_CASE("poller.loop_once() deals with libevent failures") {
    Poller poller;
    REQUIRE_THROWS(poller_loop_once<fail>(poller.base_));
}

TEST_CASE("poller.break_loop() works properly") {
    Poller poller;
    REQUIRE_THROWS(poller_break_loop<fail>(poller.base_));
}

TEST_CASE("poller.call_soon() works") {
    Poller poller;
    auto now = mk::time_now();
    poller.call_soon([&poller]() { poller.break_loop(); });
    poller.loop();
    REQUIRE((mk::time_now() - now) < 1.00); // Very conservative check
}

TEST_CASE("poller.call_later() works") {
    Poller poller;
    auto now = mk::time_now();
    poller.call_later(3.14, [&poller]() { poller.break_loop(); });
    poller.loop();
    REQUIRE((mk::time_now() - now) > 3.00); // Very conservative check
}

TEST_CASE("The periodic event is fired when we call loop()") {
    Poller poller;
    unsigned int count = 0;
    poller.on_periodic_([&count](Poller *poller) {
        if (++count < 3) {
            return;
        }
        poller->break_loop();
    });
    poller.loop();
    REQUIRE(count == 3);
}
