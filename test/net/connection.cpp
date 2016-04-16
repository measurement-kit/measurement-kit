// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

#include <measurement_kit/net.hpp>

#include "src/net/connection.hpp"
#include "src/common/check_connectivity.hpp"

using namespace mk;
using namespace mk::net;

// TODO: move this test in test/net/transport.cpp
TEST_CASE("Transport::close() is idempotent") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("nexa.polito.it", 80, [](Error err, Var<Transport> s) {
            REQUIRE(!err);
            s->close();
            // It shall be safe to call close more than once
            s->close();
            s->close();
            break_loop();
        });
    });
}

// TODO: move this test in test/net/transport.cpp
TEST_CASE("It is safe to manipulate Transport after close") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("nexa.polito.it", 80, [](Error err, Var<Transport> s) {
            REQUIRE(!err);
            s->close();
            // One should not manipulate a Transport after close, but in any
            // case that should be safe, because now close only removes all
            // possible self references and clears the bufferevent.
            s->write("foo");
            break_loop();
        });
    });
}

// TODO: remove (test not applicable anymore)
// Disable this unittest since this requires an API change
/* TEST_CASE("It is safe to close Connection while resolve is in progress") { */
/*     if (CheckConnectivity::is_down()) { */
/*         return; */
/*     } */
/*     Connection s("PF_INET", "nexa.polito.it", "80"); */
/*     DelayedCall unsched(0.001, [&s]() { s.close(); }); */
/*     DelayedCall bail_out(2.0, []() { mk::break_loop(); }); */
/*     mk::loop(); */
/* } */

TEST_CASE("connect() iterates over all the available addresses") {
    // TODO: remove
    // Test cancelled because not applicable anymore
}

TEST_CASE("It is possible to use Connection with a custom poller") {
    // TODO: remove
    // Test superseded by similar test in test/net/transport.cpp
}
