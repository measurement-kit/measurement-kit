/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/poller.cpp's IghtDelayedCall()
//

#include "src/ext/Catch/single_include/catch.hpp"
#include "common/poller.h"

#include <iostream> 
TEST_CASE("Bad allocations triggers a failure ") {
	IghtLibevent libevent;

	libevent.event_new = [](event_base*, evutil_socket_t, short,
	    event_callback_fn, void *) {
		return ((event *) NULL);
	};

	REQUIRE_THROWS_AS(IghtDelayedCall(0.0, [](void) { }, &libevent),
                    std::bad_alloc&);
}

TEST_CASE("If event_add returns -1 then an exception should be raised") {
	IghtLibevent libevent;
	libevent.event_add = [](event*, timeval *) {
		return (-1);
	};

	REQUIRE_THROWS_AS(IghtDelayedCall(0.0, [](void) { }, &libevent),
                    std::runtime_error&);

}

TEST_CASE("Check that the event callbacks are fired") {
	IghtLibevent libevent;

  SECTION("event_free must be called") {
	  auto event_free_called = false;

    libevent.event_free = [&event_free_called](event *evp) {
      event_free_called = true;
      ::event_free(evp);
    };

		IghtDelayedCall(0.0, [](void) { }, &libevent);

    REQUIRE(event_free_called == true);
  }
}


TEST_CASE("Move semantics must preserve libevent free calling") {

  SECTION("Must preserve libevent free calling") {
    auto event_free_called = 0;
    IghtLibevent libevent;

    libevent.event_free = [&event_free_called](event *evp) {
      ++event_free_called;
      ::event_free(evp);
    };

    auto d1 = IghtDelayedCall(0.0, [](void) { }, &libevent);
    {
      // Move constructor
      IghtDelayedCall d2(std::move(d1));
    }
    REQUIRE(event_free_called == 1);

    auto d3 = IghtDelayedCall(0.0, [](void) { }, &libevent);
    {
      // Move assignment
      auto d4 = IghtDelayedCall();
      d4 = std::move(d3);
    }
    REQUIRE(event_free_called == 2);
  }
  
  SECTION("Replace a delayed call with a new one") {
    auto called = false;
    auto not_called = false;

    //
    // Register a first delayed call (which will be overriden
    // later) to ensure that move semantic works.
    //
    auto d2 = IghtDelayedCall(0.0, [&](void) {
        not_called = true;
    });

    //
    // Replace the delayed call (which should clear the previous
    // delayed call contained by d2, if any) with a new one, using
    // the move semantic.
    //
    d2 = IghtDelayedCall(0.0, [&](void) {
        called = true;
        ight_break_loop();
	  });

    ight_loop();

    REQUIRE(called == true);
    REQUIRE(not_called == false);
  }
}


TEST_CASE("Destructor cancels delayed calls") {

  SECTION("Create a structure referencing an IghtDelayedCall and destroy it") {
    //
    // Make sure that the delayed call is unscheduled when the
    // object X is deleted, which simplifies life when you have
    // a delayed call bound to a connection that may be closed
    // at any time by peer.
    //
    struct X {
      IghtDelayedCall d;
    };

    auto called = false;
    auto x = new X();
    x->d = IghtDelayedCall(0.0, [&](void) {
        called = true;
    });
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
    auto d1 = new IghtDelayedCall(0.25, [&](void) {
        called = true;
    });
    auto d2 = IghtDelayedCall(0.249, [&](void) {
      delete (d1);
    });
    d1 = NULL;  /* Clear the pointer, just in case */
    REQUIRE(called == false);
  }
}

TEST_CASE("Delayed call construction") {
  
  SECTION("Create empty delayed call") {
    //
    // Make sure that an empty delayed call is successfully
    // destroyed (no segfault) when we leave the scope.
    //
    auto d1 = IghtDelayedCall();
  }

  SECTION("Create a delayed call with empty std::function") {
    //
    // Make sure that we don't raise an exception in the libevent
    // callback, when we're passed an empty std::function.
    //
    auto d3 = IghtDelayedCall(0.5, std::function<void(void)>());
  }
  
}


