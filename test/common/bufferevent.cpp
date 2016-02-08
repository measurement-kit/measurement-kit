// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <measurement_kit/common.hpp>
#include "src/common/bufferevent.hpp"

using namespace mk;

TEST_CASE("Constructors") {

    SECTION("Create a Bufferevent instance with an empty constructor") {
        auto libs = Libs();
        auto calls = 0;

        libs.bufferevent_socket_new = [&](event_base *, evutil_socket_t, int) {
            ++calls;
            return ((bufferevent *)NULL);
        };
        libs.bufferevent_free = [&](bufferevent *) { ++calls; };

        { Bufferevent b(&libs); }

        REQUIRE(calls == 0);
    }

    SECTION("An exception should be raised if bufferevent is NULL") {
        REQUIRE_THROWS_AS([](void) {
            Bufferevent b;
            auto p = (bufferevent *)b;
            (void)p;
            return;
        }(), std::runtime_error);
    }

    SECTION("Proper construction of Bufferevent") {
        auto libs = Libs();
        auto calls = 0;

        libs.bufferevent_socket_new =
            [&](event_base *b, evutil_socket_t s, int o) {
                ++calls;
                return (::bufferevent_socket_new(b, s, o));
            };
        libs.bufferevent_free = [&](bufferevent *b) {
            ++calls;
            ::bufferevent_free(b);
        };

        {
            auto poller = Poller::global();
            Bufferevent b(poller->get_event_base(), -1, 0, &libs);
        }

        REQUIRE(calls == 2);
    }

    SECTION(
        "bufferevent_socket_new return NULL results in bad_alloc exception") {
        auto libs = Libs();
        auto raised = 0;

        libs.bufferevent_socket_new = [&](event_base *, evutil_socket_t, int) {
            return ((bufferevent *)NULL);
        };

        auto base = Poller::global()->get_event_base();

        REQUIRE_THROWS_AS(Bufferevent(base, 0, 0, &libs), std::bad_alloc);
    }
}

TEST_CASE("Bufferevent operations") {

    SECTION("Accessing underlying bev") {
        auto libs = Libs();
        auto underlying = (bufferevent *)NULL;

        libs.bufferevent_socket_new =
            [&](event_base *b, evutil_socket_t s, int o) {
                return (underlying = ::bufferevent_socket_new(b, s, o));
            };

        auto poller = Poller::global();
        Bufferevent b(poller->get_event_base(), -1, 0, &libs);

        REQUIRE(underlying == (bufferevent *)b);
    }

    SECTION("The close() method is safe and idempotent") {

        auto libs = Libs();
        auto called = 0;
        libs.bufferevent_free = [&called](bufferevent *bev) {
            ++called;
            ::bufferevent_free(bev);
        };

        Bufferevent b(mk::get_global_event_base(), -1, 0, &libs);

        b.close();

        // Safe:
        REQUIRE(called == 1);
        REQUIRE_THROWS((bufferevent *)b);

        // Idempotent:
        b.close();
        b.close();
        b.close();
        REQUIRE(called == 1);
        REQUIRE_THROWS((bufferevent *)b);
    }

    SECTION("The make() method calls close() when a previous bufev exists") {

        auto libs = Libs();
        auto called = 0;
        libs.bufferevent_free = [&called](bufferevent *bev) {
            ++called;
            ::bufferevent_free(bev);
        };

        Bufferevent b(mk::get_global_event_base(), -1, 0, &libs);

        b.make(mk::get_global_event_base(), -1, 0);

        // Ensure that close() was called before creating a new bufferevent
        REQUIRE(called == 1);
    }

    SECTION("The make() method does not close() when not needed") {

        auto libs = Libs();
        auto called = 0;
        libs.bufferevent_free = [&called](bufferevent *bev) {
            ++called;
            ::bufferevent_free(bev);
        };

        Bufferevent b(&libs);

        b.make(mk::get_global_event_base(), -1, 0);

        // Ensure that close() was not called in this case
        REQUIRE(called == 0);
    }

    SECTION("The make() method calls bufferevent_socket_new()") {

        bufferevent *bevp = nullptr;
        auto libs = Libs();
        auto called = 0;
        libs.bufferevent_socket_new =
            [&bevp, &called](event_base *evb, evutil_socket_t fd, int options) {
                bevp = ::bufferevent_socket_new(evb, fd, options);
                ++called;
                return bevp;
            };

        Bufferevent b(mk::get_global_event_base(), -1, 0);

        b.make(mk::get_global_event_base(), -1, 0, &libs);

        // Ensure that bufferevent_socket_new() was called
        REQUIRE(bevp == (bufferevent *)b);
    }

    SECTION("Many subsequent make() calles behave correctly") {

        auto libs = Libs();
        auto free_called = 0;
        auto new_called = 0;
        libs.bufferevent_free = [&free_called](bufferevent *bev) {
            ++free_called;
            ::bufferevent_free(bev);
        };
        libs.bufferevent_socket_new =
            [&new_called](event_base *evb, evutil_socket_t fd, int options) {
                auto bevp = ::bufferevent_socket_new(evb, fd, options);
                ++new_called;
                return bevp;
            };

        Bufferevent b(&libs);

        b.make(mk::get_global_event_base(), -1, 0, &libs);
        b.make(mk::get_global_event_base(), -1, 0, &libs);
        b.make(mk::get_global_event_base(), -1, 0, &libs);

        REQUIRE(new_called == 3);
        REQUIRE(free_called == 2);
    }

    SECTION("The make() method allows to change the underlying libs") {
        auto libs = Libs();

        Bufferevent b(&libs);

        b.make(mk::get_global_event_base(), -1, 0);

        // Check whether the libs is changed as it ought to be
        REQUIRE(b.get_libs() == Libs::global());

        auto libs2 = Libs();
        b.make(mk::get_global_event_base(), -1, 0, &libs2);

        // Check whether the libs is changed as it ought to be
        REQUIRE(b.get_libs() == &libs2);
    }
}
