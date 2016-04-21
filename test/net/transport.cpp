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
            Var<ConnectResult> cr = error.context.as<ConnectResult>();
            REQUIRE(cr);
            REQUIRE(!cr->resolve_result.inet_pton_ipv4);
            REQUIRE(!cr->resolve_result.inet_pton_ipv6);
            REQUIRE(cr->resolve_result.addresses.size() > 0);
            REQUIRE(cr->connected_bev);
            txp->close([]() { break_loop(); });
        });
    });
}

TEST_CASE("net::connect() can connect to ssl port") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    set_verbose(1);
    loop_with_initial_event([]() {
        connect("nexa.polito.it", 443, [](Error error, Var<Transport> txp) {
            REQUIRE(!error);
            Var<ConnectResult> cr = error.context.as<ConnectResult>();
            REQUIRE(cr);
            REQUIRE(!cr->resolve_result.inet_pton_ipv4);
            REQUIRE(!cr->resolve_result.inet_pton_ipv6);
            REQUIRE(cr->resolve_result.addresses.size() > 0);
            REQUIRE(cr->connected_bev);
            txp->close([]() { break_loop(); });
        },
        {{"ssl", true}, {"ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl fails when presented an expired certificate") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    set_verbose(1);
    loop_with_initial_event([]() {
        connect("expired.badssl.com", 443, [](Error error, Var<Transport> txp) {
            REQUIRE(error);
            break_loop();
        },
        {{"ssl", true}, {"ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl fails when presented a certificate with the wrong hostname") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    set_verbose(1);
    loop_with_initial_event([]() {
        connect("wrong.host.badssl.com", 443, [](Error error, Var<Transport> txp) {
            REQUIRE(error);
            break_loop();
        },
        {{"ssl", true}, {"ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl works when using SNI") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    set_verbose(1);
    loop_with_initial_event([]() {
        connect("sha256.badssl.com", 443, [](Error error, Var<Transport> txp) {
            REQUIRE(!error);
            txp->close([]() { break_loop(); });
        },
        {{"ssl", true}, {"ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() works in case of error") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("nexa.polito.it", 81, [](Error error, Var<Transport>) {
            REQUIRE(error);
            Var<ConnectResult> cr = error.context.as<ConnectResult>();
            REQUIRE(cr);
            REQUIRE(!cr->resolve_result.inet_pton_ipv4);
            REQUIRE(!cr->resolve_result.inet_pton_ipv6);
            REQUIRE(cr->resolve_result.addresses.size() > 0);
            REQUIRE(!cr->connected_bev);
            break_loop();
        }, {{"timeout", 5.0}});
    });
}
