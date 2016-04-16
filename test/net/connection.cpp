// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/net/connection.h's Connection{State,}
//

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
            // We expect the transport to throw if we call its ->write() method
            // after close because the underlying Bufferevent becomes empty
            REQUIRE_THROWS(s->write("foo"));
            break_loop();
        });
    });
}

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
    if (CheckConnectivity::is_down()) {
        return;
    }
    Connection s("PF_UNSPEC", "www.youtube.com", "81");
    s.set_timeout(5);
    s.on_error([](Error error) {
        auto ok = (error == SocketError() || error == ConnectFailedError());
        REQUIRE(ok);
        mk::break_loop();
    });
    mk::loop();
}

TEST_CASE("It is possible to use Connection with a custom poller") {
    // Note: this is how Portolan uses measurement-kit
    if (CheckConnectivity::is_down()) {
        return;
    }
    Poller poller;
    Connection s("PF_UNSPEC", "nexa.polito.it", "22", Logger::global(),
                 &poller);
    s.set_timeout(5);
    auto ok = false;
    s.on_error([&poller](Error) { poller.break_loop(); });
    s.on_connect([&poller, &ok]() {
        poller.break_loop();
        ok = true;
    });
    poller.loop();
    REQUIRE(ok);
}
