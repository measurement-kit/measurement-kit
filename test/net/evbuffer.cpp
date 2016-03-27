// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/net/evbuffer.hpp"
#include "src/common/delayed_call.hpp"

using namespace mk::net;
using namespace mk;

TEST_CASE("The constructor is lazy") {

    auto libs = Libs();
    auto calls = 0;

    libs.evbuffer_new = [&](void) {
        ++calls;
        return ((evbuffer *)NULL);
    };
    libs.evbuffer_free = [&](evbuffer *) { ++calls; };

    { Evbuffer evbuf(&libs); }

    REQUIRE(calls == 0);
}

TEST_CASE("The (evbuffer*) operation allocates the internal evbuffer") {

    auto libs = Libs();
    auto calls = 0;

    libs.evbuffer_new = [&](void) {
        ++calls;
        return (::evbuffer_new());
    };
    libs.evbuffer_free = [&](evbuffer *e) {
        ++calls;
        evbuffer_free(e);
    };

    {
        Evbuffer evbuf(&libs);
        auto p = (evbuffer *)evbuf;
        (void)p;
    }

    REQUIRE(calls == 2);
}

TEST_CASE("The (evbuffer *) operation is idempotent") {

    auto libs = Libs();
    auto calls = 0;

    libs.evbuffer_new = [&](void) {
        ++calls;
        return (::evbuffer_new());
    };

    Evbuffer evbuf(&libs);
    auto p1 = (evbuffer *)evbuf;
    auto p2 = (evbuffer *)evbuf;

    REQUIRE(p1 == p2);
    REQUIRE(calls == 1);
}

TEST_CASE("std::bad_alloc is raised when out of memory") {

    auto libs = Libs();

    libs.evbuffer_new = [&](void) { return ((evbuffer *)NULL); };

    REQUIRE_THROWS_AS([&](void) {
        /* Yes, I really really love inline functions <3 */
        Evbuffer evbuf(&libs);
        auto p = (evbuffer *)evbuf;
        (void)p;
    }(), std::bad_alloc);
}
