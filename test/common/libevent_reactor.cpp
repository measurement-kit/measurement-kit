// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/libevent_reactor.hpp"
#include "private/common/utils.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;

extern "C" {

static int evthread_use_pthreads_fail() { return -1; }
static int sigaction_fail(int, const struct sigaction *, struct sigaction *) {
    return -1;
}

} // extern "C"

TEST_CASE("libevent_init_once") {
    SECTION("We deal with evthread_use_pthreads() failure") {
        REQUIRE_THROWS((LibeventReactor<>::libevent_init_once<
                evthread_use_pthreads_fail, sigaction>()));
    }

    SECTION("We deal with sigaction() failure") {
        REQUIRE_THROWS(
                (LibeventReactor<>::libevent_init_once<evthread_use_pthreads,
                        sigaction_fail>()));
    }
}

extern "C" {

static event_base *event_base_new_fail() { return nullptr; }

static int event_base_dispatch_fail(event_base *) { return -1; }

static int event_base_dispatch_no_events(event_base *) { return 1; }

static int event_base_loopbreak_fail(event_base *) { return -1; }

} // extern "C"

TEST_CASE("Reactor: basic functionality") {
    SECTION("We deal with event_base_new() failure") {
        REQUIRE_THROWS((LibeventReactor<event_base_new_fail, event_base_once,
                event_base_dispatch, event_base_loopbreak, event_new,
                event_add>{}));
    }

    SECTION("We deal with event_base_dispatch() failure") {
        LibeventReactor<event_base_new, event_base_once,
                event_base_dispatch_fail, event_base_loopbreak, event_new,
                event_add>
                reactor;
        REQUIRE_THROWS(reactor.run());
    }

    SECTION("We deal with event_base_dispatch() running out of events") {
        LibeventReactor<event_base_new, event_base_once,
                event_base_dispatch_no_events, event_base_loopbreak, event_new,
                event_add>
                reactor;
        reactor.run();
    }

    SECTION("We deal with event_base_loopbreak() failure") {
        LibeventReactor<event_base_new, event_base_once, event_base_dispatch,
                event_base_loopbreak_fail, event_new, event_add>
                reactor;
        REQUIRE_THROWS(reactor.stop());
    }
}

extern "C" {

static int event_base_once_fail(event_base *, evutil_socket_t, short,
        event_callback_fn, void *, const timeval *) {
    return -1;
}

} // extern "C"

TEST_CASE("Reactor: call_later") {
    SECTION("We deal with event_base_once() failure") {
        LibeventReactor<event_base_new, event_base_once_fail,
                event_base_dispatch, event_base_loopbreak>
                reactor;
        REQUIRE_THROWS(reactor.call_later(0.0, []() {}));
    }
}

TEST_CASE("Reactor: pollfd") {
    SECTION("We deal with event_base_once() failure") {
        LibeventReactor<event_base_new, event_base_once_fail,
                event_base_dispatch, event_base_loopbreak>
                reactor;
        REQUIRE_THROWS(reactor.pollfd(0, 0, 0.0, [](Error, short) {}));
    }
}
