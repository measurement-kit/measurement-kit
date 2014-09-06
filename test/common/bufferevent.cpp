/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's IghtBufferevent
//

#include "src/ext/Catch/single_include/catch.hpp"
#include "common/poller.h"

#include <iostream>

TEST_CASE("Constructors") {
  
  SECTION("Create a IghtLibevent instance with an empty constructor") {
    auto libevent = IghtLibevent();
    auto calls = 0;

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
    
    REQUIRE(calls == 0);
  }

  SECTION("An exception should be raised if bufferevent is NULL") {
    REQUIRE_THROWS_AS([](void) {
        auto b = IghtBuffereventSocket();
        auto p = (bufferevent *) b;
        (void) p;
        return;
    }(), std::runtime_error);
  }

  SECTION("Proper construction of IghtLibevent") {
    auto libevent = IghtLibevent();
    auto calls = 0;

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

    REQUIRE(calls == 2);
  }

  SECTION("bufferevent_socket_new return NULL results in bad_alloc exception") {
    auto libevent = IghtLibevent();
    auto raised = 0;

    libevent.bufferevent_socket_new = [&](event_base *,
        evutil_socket_t, int) {
      return ((bufferevent *) NULL);
    };

    auto base = IghtGlobalPoller::get()->get_event_base();

    REQUIRE_THROWS_AS(IghtBuffereventSocket(base, 0, 0, &libevent),
                      std::bad_alloc);
  }
}

TEST_CASE("IghtBufferEventSocket operations") {

  SECTION("Accessing underlying bev") {
    auto libevent = IghtLibevent();
    auto underlying = (bufferevent *) NULL;

    libevent.bufferevent_socket_new = [&](event_base *b,
        evutil_socket_t s, int o) {
      return (underlying = ::bufferevent_socket_new(b, s, o));
    };

    auto poller = IghtGlobalPoller::get();
    auto b = IghtBuffereventSocket(poller->get_event_base(), -1, 0,
        &libevent);

    REQUIRE(underlying == (bufferevent *) b);
  }
  
  SECTION("Move constructor") {
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

    REQUIRE(b2.get_libevent() == &libevent);
    REQUIRE(underlying == (bufferevent *) b2);
    REQUIRE(b1.get_libevent() == IghtGlobalLibevent::get());

    REQUIRE_THROWS_AS([&](void){
      auto ppp = (bufferevent *) b1;
      (void) ppp;
    }(), std::runtime_error);

  }

  SECTION("Move assignment") {
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

    REQUIRE(b2.get_libevent() == &libevent1);
    REQUIRE(underlying == (bufferevent *) b2);
    REQUIRE(b1.get_libevent() == &libevent2);

    REQUIRE_THROWS_AS([&](void){
      auto ppp = (bufferevent *) b1;
      (void) ppp;
    }(), std::runtime_error);

  }
}
