/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's Bufferevent
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/poller.hpp>

using namespace ight::common::libevent;
using namespace ight::common::poller;

TEST_CASE("Constructors") {
  
  SECTION("Create a BuffereventSocket instance with an empty constructor") {
    auto libevent = Libevent();
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
      BuffereventSocket b(&libevent);
    }
    
    REQUIRE(calls == 0);
  }

  SECTION("An exception should be raised if bufferevent is NULL") {
    REQUIRE_THROWS_AS([](void) {
        BuffereventSocket b;
        auto p = (bufferevent *) b;
        (void) p;
        return;
    }(), std::runtime_error);
  }

  SECTION("Proper construction of BuffereventSocket") {
    auto libevent = Libevent();
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
      auto poller = GlobalPoller::get();
      BuffereventSocket b(poller->get_event_base(),
          -1, 0, &libevent);
    }

    REQUIRE(calls == 2);
  }

  SECTION("bufferevent_socket_new return NULL results in bad_alloc exception") {
    auto libevent = Libevent();
    auto raised = 0;

    libevent.bufferevent_socket_new = [&](event_base *,
        evutil_socket_t, int) {
      return ((bufferevent *) NULL);
    };

    auto base = GlobalPoller::get()->get_event_base();

    REQUIRE_THROWS_AS(BuffereventSocket(base, 0, 0, &libevent),
                      std::bad_alloc);
  }
}

TEST_CASE("BuffereventSocket operations") {

  SECTION("Accessing underlying bev") {
    auto libevent = Libevent();
    auto underlying = (bufferevent *) NULL;

    libevent.bufferevent_socket_new = [&](event_base *b,
        evutil_socket_t s, int o) {
      return (underlying = ::bufferevent_socket_new(b, s, o));
    };

    auto poller = GlobalPoller::get();
    BuffereventSocket b(poller->get_event_base(), -1, 0,
        &libevent);

    REQUIRE(underlying == (bufferevent *) b);
  }

  SECTION("The close() method is safe and idempotent") {

    auto libevent = Libevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    BuffereventSocket b(ight_get_global_event_base(), -1,
        0, &libevent);

    b.close();

    // Safe:
    bufferevent *come_fosse_antani;
    REQUIRE(called == 1);
    REQUIRE_THROWS(come_fosse_antani = (bufferevent *) b);

    // Idempotent:
    b.close();
    b.close();
    b.close();
    REQUIRE(called == 1);
    REQUIRE_THROWS(come_fosse_antani = (bufferevent *) b);
  }

  SECTION("The make() method calls close() when a previous bufev exists") {

    auto libevent = Libevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    BuffereventSocket b(ight_get_global_event_base(), -1,
        0, &libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Ensure that close() was called before creating a new bufferevent
    REQUIRE(called == 1);
  }

  SECTION("The make() method does not close() when not needed") {

    auto libevent = Libevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    BuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Ensure that close() was not called in this case
    REQUIRE(called == 0);
  }

  SECTION("The make() method calls bufferevent_socket_new()") {

    bufferevent *bevp = nullptr;
    auto libevent = Libevent();
    auto called = 0;
    libevent.bufferevent_socket_new = [&bevp, &called](event_base *evb,
        evutil_socket_t fd, int options) {
      bevp = ::bufferevent_socket_new(evb, fd, options);
      ++called;
      return bevp;
    };

    BuffereventSocket b(ight_get_global_event_base(), -1, 0);

    b.make(ight_get_global_event_base(), -1, 0, &libevent);

    // Ensure that bufferevent_socket_new() was called
    REQUIRE(bevp == (bufferevent *) b);
  }

  SECTION("Many subsequent make() calles behave correctly") {

    auto libevent = Libevent();
    auto free_called = 0;
    auto new_called = 0;
    libevent.bufferevent_free = [&free_called](bufferevent *bev) {
      ++free_called;
      ::bufferevent_free(bev);
    };
    libevent.bufferevent_socket_new = [&new_called](event_base *evb,
        evutil_socket_t fd, int options) {
      auto bevp = ::bufferevent_socket_new(evb, fd, options);
      ++new_called;
      return bevp;
    };

    BuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0, &libevent);
    b.make(ight_get_global_event_base(), -1, 0, &libevent);
    b.make(ight_get_global_event_base(), -1, 0, &libevent);

    REQUIRE(new_called == 3);
    REQUIRE(free_called == 2);
  }

  SECTION("The make() method allows to change the underlying libevent") {
    auto libevent = Libevent();

    BuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Check whether the libevent is changed as it ought to be
    REQUIRE(b.get_libevent() == GlobalLibevent::get());

    auto libevent2 = Libevent();
    b.make(ight_get_global_event_base(), -1, 0, &libevent2);

    // Check whether the libevent is changed as it ought to be
    REQUIRE(b.get_libevent() == &libevent2);
  }

}
