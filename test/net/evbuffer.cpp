// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/net/evbuffer.hpp"

using namespace mk::net;
using namespace mk;

static evbuffer *fail() { return nullptr; }

TEST_CASE("make_shared_evbuffer deals with evbuffer_new() failure") {
    REQUIRE_THROWS_AS([](){ make_shared_evbuffer<fail>(); }(),
                      std::bad_alloc);
}

static bool ctor_called = false;
static evbuffer *ctor() {
    ctor_called = true;
    return (evbuffer *)0xabad1dea;
}

static bool dtor_called = false;
static void dtor(evbuffer *p) {
    dtor_called = true;
    REQUIRE(p == (evbuffer *)0xabad1dea);
}

TEST_CASE("make_shared_evbuffer creates a SharedPtr where evbuffer_free is called "
          "when the last SharedPtr is gone") {
    REQUIRE(ctor_called == false);
    REQUIRE(dtor_called == false);
    { make_shared_evbuffer<ctor, dtor>(); }
    REQUIRE(ctor_called == true);
    REQUIRE(dtor_called == true);
}

TEST_CASE("SharedPtr<evbuffer> works as expected") {
    // This test is mean to check whether there are leaks or not using Valgrind
    SharedPtr<evbuffer> evbuf = make_shared_evbuffer();
    REQUIRE(evbuf.get() != nullptr);
}
