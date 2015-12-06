// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/poller.cpp's DelayedCall()
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/common/delayed_call.hpp"

using namespace mk;

TEST_CASE("Bad allocations triggers a failure ") {
    Libs libs;

    libs.event_new = [](event_base *, evutil_socket_t, short, event_callback_fn,
                        void *) { return ((event *)NULL); };

    REQUIRE_THROWS_AS(DelayedCall(0.0, [](void) {}, &libs), std::bad_alloc &);
}

TEST_CASE("If event_add returns -1 then an exception should be raised") {
    Libs libs;
    libs.event_add = [](event *, timeval *) { return (-1); };

    REQUIRE_THROWS_AS(DelayedCall(0.0, [](void) {}, &libs),
                      std::runtime_error &);
}

TEST_CASE("Check that the event callbacks are fired") {
    Libs libs;

    SECTION("event_free must be called") {
        auto event_free_called = false;

        libs.event_free = [&event_free_called](event *evp) {
            event_free_called = true;
            ::event_free(evp);
        };

        DelayedCall(0.0, [](void) {}, &libs);

        REQUIRE(event_free_called == true);
    }
}

TEST_CASE("Destructor cancels delayed calls") {

    SECTION("Create a structure referencing an DelayedCall and destroy it") {
        //
        // Make sure that the delayed call is unscheduled when the
        // object X is deleted, which simplifies life when you have
        // a delayed call bound to a connection that may be closed
        // at any time by peer.
        //
        struct X {
            Var<DelayedCall> d;
        };

        auto called = false;
        auto x = new X();
        x->d = std::make_shared<DelayedCall>(0.0, [&](void) { called = true; });
        delete (x);
        REQUIRE(called == false);
    }

    SECTION("Create two delayed calls the first killing off the second") {
        //
        // Same as above, but this time cancelling the callback
        // right before it is due.
        //
        // This models the case in which an asynchronous event, e.g.
        // a FIN packet, triggers the deletion of an object that
        // contains a delayed call that, in turn, is about to run.
        //
        // In such case, we want the delayed call not to run, no
        // matter how close the deadline is.
        //
        auto called = false;
        auto d1 = new DelayedCall(0.25, [&](void) { called = true; });
        DelayedCall d2(0.249, [&](void) { delete (d1); });
        DelayedCall d3(0.33, []() { mk::break_loop(); });
        mk::loop();
        d1 = NULL; /* Clear the pointer, just in case */
        REQUIRE(called == false);
    }
}

TEST_CASE("Delayed call construction") {

    SECTION("Create empty delayed call") {
        //
        // Make sure that an empty delayed call is successfully
        // destroyed (no segfault) when we leave the scope.
        //
        DelayedCall d1{0.0, []() {}};
    }

    SECTION("Create a delayed call with empty std::function") {
        //
        // Make sure that we don't raise an exception in the libevent
        // callback, when we're passed an empty std::function.
        //
        DelayedCall d3(0.0, std::function<void(void)>());
        DelayedCall d4(0.1, []() { mk::break_loop(); });
        mk::loop();
    }
}
