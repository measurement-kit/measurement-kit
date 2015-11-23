// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/poller.cpp's Poller()
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/common/delayed_call.hpp"

#include <functional>
#include <new>
#include <stdexcept>

using namespace measurement_kit::common;

TEST_CASE("Constructor") {

    SECTION("We deal with event_base_new() failure") {
        auto libs = Libs();

        libs.event_base_new = [](void) { return ((event_base *)nullptr); };

        auto bad_alloc_fired = false;
        try {
            Poller poller(&libs);
        } catch (std::bad_alloc &) {
            bad_alloc_fired = true;
        }

        REQUIRE(bad_alloc_fired);
    }

    SECTION("We deal with evdns_base_new() failure") {
        auto libs = Libs();

        auto event_base_free_fired = false;

        libs.event_base_free = [&event_base_free_fired](event_base *b) {
            event_base_free_fired = true;
            ::event_base_free(b);
        };
        libs.evdns_base_new =
            [](event_base *, int) { return ((evdns_base *)nullptr); };

        auto bad_alloc_fired = false;
        try {
            Poller poller(&libs);
        } catch (std::bad_alloc &) {
            bad_alloc_fired = true;
        }

        REQUIRE(bad_alloc_fired);
        REQUIRE(event_base_free_fired);
    }
}

TEST_CASE("The destructor works properly") {

    auto libs = Libs();

    auto event_base_free_fired = false;
    auto evdns_base_free_fired = false;

    libs.event_base_free = [&event_base_free_fired](event_base *b) {
        event_base_free_fired = true;
        ::event_base_free(b);
    };
    libs.evdns_base_free = [&evdns_base_free_fired](evdns_base *b, int opt) {
        evdns_base_free_fired = true;
        ::evdns_base_free(b, opt);
    };

    { Poller poller(&libs); }

    REQUIRE(event_base_free_fired);
    REQUIRE(evdns_base_free_fired);
}

TEST_CASE("poller.loop() works properly in corner cases") {

    SECTION("We deal with event_base_dispatch() returning -1") {
        Libs libs;

        libs.event_base_dispatch = [](event_base *) { return (-1); };

        Poller poller1(&libs);

        auto runtime_error_fired = false;
        try {
            poller1.loop();
        } catch (std::runtime_error &) {
            runtime_error_fired = true;
        }

        REQUIRE(runtime_error_fired);
    }

    SECTION("We deal with event_base_dispatch() returning 1") {
        Libs libs;

        libs.event_base_dispatch = [](event_base *) { return (1); };
        Poller poller2(&libs);
        poller2.loop();
    }
}

TEST_CASE("poller.break_loop() works properly") {
    Libs libs;

    libs.event_base_loopbreak = [](event_base *) { return (-1); };

    Poller poller(&libs);

    auto runtime_error_fired = false;
    try {
        poller.break_loop();
    } catch (std::runtime_error &) {
        runtime_error_fired = true;
    }

    REQUIRE(runtime_error_fired);
}
