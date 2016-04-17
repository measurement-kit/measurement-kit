// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

#include "src/common/check_connectivity.hpp"

#include <iostream>

using namespace mk;
using namespace mk::net;

TEST_CASE("net::connect() works with a custom poller") {
    // Note: this is how Portolan uses measurement-kit
    if (CheckConnectivity::is_down()) {
        return;
    }
    set_verbose(1);
    Poller poller;
    auto ok = false;
    connect("nexa.polito.it", 22, [&](Error error, Var<Transport> txp) {
        if (error) {
            poller.break_loop();
            return;
        }
        txp->close([&]() { poller.break_loop(); });
        ok = true;
    }, {}, Logger::global(), &poller);
    poller.loop();
    REQUIRE(ok);
}

TEST_CASE("net::connect() can connect to open port") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("www.kernel.org", 80, [](Error error, Var<Transport> txp) {
            REQUIRE(!error);
            txp->close([]() { break_loop(); });
        });
    });
}

TEST_CASE("net::connect() works in case of error") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("nexa.polito.it", 81, [](Error error, Var<Transport>) {
            REQUIRE(error);
            break_loop();
        }, {{"timeout", 5.0}});
    });
}
