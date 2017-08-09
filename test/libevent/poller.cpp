// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/utils.hpp"
#include "private/libevent/poller.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;
using namespace mk::libevent;

extern "C" {

static int evthread_use_pthreads_fail() { return -1; }
static int sigaction_fail(int, const struct sigaction *, struct sigaction *) {
    return -1;
}

} // extern "C"

TEST_CASE("LibeventLibrary") {
    SECTION("We deal with evthread_use_pthreads() failure") {
        REQUIRE_THROWS((Poller<>::LibeventLibrary<evthread_use_pthreads_fail,
                                                 sigaction>{}));
    }

    SECTION("We deal with sigaction() failure") {
        REQUIRE_THROWS((Poller<>::LibeventLibrary<evthread_use_pthreads,
                                                 sigaction_fail>{}));
    }
}

extern "C" {

static event_base *event_base_new_fail() { return nullptr; }

static int event_base_dispatch_fail(event_base *) { return -1; }

static int event_base_dispatch_no_events(event_base *) { return 1; }

static int event_base_loopbreak_fail(event_base *) { return -1; }

} // extern "C"

TEST_CASE("Poller: basic functionality") {
    SECTION("We deal with event_base_new() failure") {
        REQUIRE_THROWS((Poller<event_base_new_fail, event_base_once,
                               event_base_dispatch, event_base_loopbreak,
                               event_new, event_add>{}));
    }

    SECTION("We deal with event_base_dispatch() failure") {
        Poller<event_base_new, event_base_once, event_base_dispatch_fail,
               event_base_loopbreak, event_new, event_add>
              poller;
        REQUIRE_THROWS(poller.run());
    }

    SECTION("We deal with event_base_dispatch() running out of events") {
        Poller<event_base_new, event_base_once, event_base_dispatch_no_events,
               event_base_loopbreak, event_new, event_add>
              poller;
        poller.run();
    }

    SECTION("We deal with event_base_loopbreak() failure") {
        Poller<event_base_new, event_base_once, event_base_dispatch,
               event_base_loopbreak_fail, event_new, event_add>
              poller;
        REQUIRE_THROWS(poller.stop());
    }
}

extern "C" {

static event *event_new_fail(event_base *, evutil_socket_t, short,
                             event_callback_fn, void *) {
    return nullptr;
}

static int event_add_fail(event *, const timeval *) {
    return -1;
}

} // extern "C"

TEST_CASE("Poller: periodic event") {
    SECTION("We deal with event_new() failure") {
        Poller<event_base_new, event_base_once, event_base_dispatch_no_events,
               event_base_loopbreak, event_new_fail, event_add>
              poller;
        REQUIRE_THROWS(poller.run());
    }

    SECTION("We deal with event_add() failure") {
        Poller<event_base_new, event_base_once, event_base_dispatch_no_events,
               event_base_loopbreak, event_new, event_add_fail>
              poller;
        REQUIRE_THROWS(poller.run());
    }
}

extern "C" {

static int event_base_once_fail(event_base *, evutil_socket_t, short,
                                event_callback_fn, void *, const timeval *) {
    return -1;
}

} // extern "C"

TEST_CASE("Poller: call_later") {
    SECTION("We deal with event_base_once() failure") {
        Poller<event_base_new, event_base_once_fail, event_base_dispatch,
               event_base_loopbreak>
              poller;
        REQUIRE_THROWS(poller.call_later(0.0, []() {}));
    }

    SECTION("We don't leak if the callback throws an exception") {
        Poller<> poller;
        poller.call_later(0.1, []() { throw std::exception{}; });
        REQUIRE_THROWS(poller.run());
    }
}

TEST_CASE("Poller: pollfd") {
    SECTION("We deal with event_base_once() failure") {
        Poller<event_base_new, event_base_once_fail, event_base_dispatch,
               event_base_loopbreak>
              poller;
        REQUIRE_THROWS(poller.pollfd(0, 0, 0.0, [](Error, short) {}));
    }

    SECTION("We don't leak if the callback throws an exception") {
        Poller<> poller;
        poller.pollfd(0, 0, 0.1, [](Error, short) { throw std::exception{}; });
        REQUIRE_THROWS(poller.run());
    }
}
