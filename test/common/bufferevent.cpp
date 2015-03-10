/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/libevent.h's IghtBufferevent
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/common/poller.h>

TEST_CASE("Constructors") {
  
  SECTION("Create a IghtBuffereventSocket instance with an empty constructor") {
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
      IghtBuffereventSocket b(&libevent);
    }
    
    REQUIRE(calls == 0);
  }

  SECTION("An exception should be raised if bufferevent is NULL") {
    REQUIRE_THROWS_AS([](void) {
        IghtBuffereventSocket b;
        auto p = (bufferevent *) b;
        (void) p;
        return;
    }(), std::runtime_error);
  }

  SECTION("Proper construction of IghtBuffereventSocket") {
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
      IghtBuffereventSocket b(poller->get_event_base(),
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

TEST_CASE("IghtBuffereventSocket operations") {

  SECTION("Accessing underlying bev") {
    auto libevent = IghtLibevent();
    auto underlying = (bufferevent *) NULL;

    libevent.bufferevent_socket_new = [&](event_base *b,
        evutil_socket_t s, int o) {
      return (underlying = ::bufferevent_socket_new(b, s, o));
    };

    auto poller = IghtGlobalPoller::get();
    IghtBuffereventSocket b(poller->get_event_base(), -1, 0,
        &libevent);

    REQUIRE(underlying == (bufferevent *) b);
  }

  SECTION("The close() method is safe and idempotent") {

    auto libevent = IghtLibevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    IghtBuffereventSocket b(ight_get_global_event_base(), -1,
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

    auto libevent = IghtLibevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    IghtBuffereventSocket b(ight_get_global_event_base(), -1,
        0, &libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Ensure that close() was called before creating a new bufferevent
    REQUIRE(called == 1);
  }

  SECTION("The make() method does not close() when not needed") {

    auto libevent = IghtLibevent();
    auto called = 0;
    libevent.bufferevent_free = [&called](bufferevent *bev) {
      ++called;
      ::bufferevent_free(bev);
    };

    IghtBuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Ensure that close() was not called in this case
    REQUIRE(called == 0);
  }

  SECTION("The make() method calls bufferevent_socket_new()") {

    bufferevent *bevp = nullptr;
    auto libevent = IghtLibevent();
    auto called = 0;
    libevent.bufferevent_socket_new = [&bevp, &called](event_base *evb,
        evutil_socket_t fd, int options) {
      bevp = ::bufferevent_socket_new(evb, fd, options);
      ++called;
      return bevp;
    };

    IghtBuffereventSocket b(ight_get_global_event_base(), -1, 0);

    b.make(ight_get_global_event_base(), -1, 0, &libevent);

    // Ensure that bufferevent_socket_new() was called
    REQUIRE(bevp == (bufferevent *) b);
  }

  SECTION("Many subsequent make() calles behave correctly") {

    auto libevent = IghtLibevent();
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

    IghtBuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0, &libevent);
    b.make(ight_get_global_event_base(), -1, 0, &libevent);
    b.make(ight_get_global_event_base(), -1, 0, &libevent);

    REQUIRE(new_called == 3);
    REQUIRE(free_called == 2);
  }

  SECTION("The make() method allows to change the underlying libevent") {
    auto libevent = IghtLibevent();

    IghtBuffereventSocket b(&libevent);

    b.make(ight_get_global_event_base(), -1, 0);

    // Check whether the libevent is changed as it ought to be
    REQUIRE(b.get_libevent() == IghtGlobalLibevent::get());

    auto libevent2 = IghtLibevent();
    b.make(ight_get_global_event_base(), -1, 0, &libevent2);

    // Check whether the libevent is changed as it ought to be
    REQUIRE(b.get_libevent() == &libevent2);
  }

}
