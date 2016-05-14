// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/common/check_connectivity.hpp"
#include <measurement_kit/net.hpp>
using namespace mk;
using namespace mk::net;

TEST_CASE("net::connect() works with a custom reactor") {
    // Note: this is how Portolan uses measurement-kit
    if (!CheckConnectivity::is_down()) {
        set_verbosity(MK_LOG_DEBUG);
        Var<Reactor> reactor = Reactor::make();
        auto ok = false;
        connect("nexa.polito.it", 22,
                [&](Error error, Var<Transport> txp) {
                    if (error) {
                        reactor->break_loop();
                        return;
                    }
                    txp->close([&]() { reactor->break_loop(); });
                    ok = true;
                },
                {}, Logger::global(), reactor);
        reactor->loop();
        REQUIRE(ok);
    }
}

TEST_CASE("net::connect() can connect to open port") {
    loop_with_initial_event_and_connectivity([]() {
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
    loop_with_initial_event_and_connectivity([]() {
        set_verbosity(MK_LOG_DEBUG);
        connect("nexa.polito.it", 443,
                [](Error error, Var<Transport> txp) {
                    REQUIRE(!error);
                    Var<ConnectResult> cr = error.context.as<ConnectResult>();
                    REQUIRE(cr);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv4);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv6);
                    REQUIRE(cr->resolve_result.addresses.size() > 0);
                    REQUIRE(cr->connected_bev);
                    txp->close([]() { break_loop(); });
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl fails when presented an expired certificate") {
    loop_with_initial_event_and_connectivity([]() {
        set_verbosity(MK_LOG_DEBUG);
        connect("expired.badssl.com", 443,
                [](Error error, Var<Transport> txp) {
                    REQUIRE(error);
                    break_loop();
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl fails when presented a certificate with the "
          "wrong hostname") {
    loop_with_initial_event_and_connectivity([]() {
        set_verbosity(MK_LOG_DEBUG);
        connect("wrong.host.badssl.com", 443,
                [](Error error, Var<Transport> txp) {
                    REQUIRE(error);
                    break_loop();
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl works when using SNI") {
    loop_with_initial_event_and_connectivity([]() {
        set_verbosity(MK_LOG_DEBUG);
        connect("sha256.badssl.com", 443,
                [](Error error, Var<Transport> txp) {
                    REQUIRE(!error);
                    txp->close([]() { break_loop(); });
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/certs.pem"}});
    });
}

TEST_CASE("net::connect() ssl works when setting ca_bundle_path via global "
          "settings") {
    set_verbosity(MK_LOG_DEBUG);
    Var<Settings> global_settings = Settings::global();
    (*global_settings)["net/ca_bundle_path"] = "test/fixtures/certs.pem";
    loop_with_initial_event_and_connectivity([]() {
        connect("nexa.polito.it", 443,
                [](Error error, Var<Transport> txp) {
                    REQUIRE(!error);
                    Var<ConnectResult> cr = error.context.as<ConnectResult>();
                    REQUIRE(cr);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv4);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv6);
                    REQUIRE(cr->resolve_result.addresses.size() > 0);
                    REQUIRE(cr->connected_bev);
                    txp->close([]() { break_loop(); });
                },
                {{"net/ssl", true}});
    });
}

TEST_CASE("net::connect() works in case of error") {
    loop_with_initial_event_and_connectivity([]() {
        connect("nexa.polito.it", 81,
                [](Error error, Var<Transport>) {
                    REQUIRE(error);
                    Var<ConnectResult> cr = error.context.as<ConnectResult>();
                    REQUIRE(cr);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv4);
                    REQUIRE(!cr->resolve_result.inet_pton_ipv6);
                    REQUIRE(cr->resolve_result.addresses.size() > 0);
                    REQUIRE(!cr->connected_bev);
                    break_loop();
                },
                {{"net/timeout", 5.0}});
    });
}
