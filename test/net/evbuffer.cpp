// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/net/evbuffer.hpp"
#include <measurement_kit/common.hpp>

using namespace mk::net;
using namespace mk;

static evbuffer *fail() { return nullptr; }

TEST_CASE("make_shared_evbuffer deals with evbuffer_new() failure") {
    REQUIRE_THROWS_AS({ make_shared_evbuffer<fail>(); }, std::bad_alloc);
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

TEST_CASE("make_shared_evbuffer creates a Var where evbuffer_free is called "
          "when the last Var is gone") {
    REQUIRE(ctor_called == false);
    REQUIRE(dtor_called == false);
    { make_shared_evbuffer<ctor, dtor>(); }
    REQUIRE(ctor_called == true);
    REQUIRE(dtor_called == true);
}

TEST_CASE("Var<evbuffer> works as expected") {
    // This test is mean to check whether there are leaks or not using Valgrind
    Var<evbuffer> evbuf = make_shared_evbuffer();
    REQUIRE(evbuf.get() != nullptr);
}
