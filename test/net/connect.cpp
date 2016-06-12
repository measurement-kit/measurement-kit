// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/common/check_connectivity.hpp"
#include "src/net/connect_impl.hpp"
#include "src/net/emitter.hpp"
#include "src/ext/Catch/single_include/catch.hpp"
#include <event2/bufferevent.h>
#include <iostream>
#include <measurement_kit/net.hpp>
using namespace mk;
using namespace mk::net;

struct bufferevent;

/*
             _ _
 _   _ _ __ (_) |_
| | | | '_ \| | __|
| |_| | | | | | |_
 \__,_|_| |_|_|\__|

*/

static int fail(const char *, sockaddr *, int *) { return -1; }

TEST_CASE("connect_base deals with evutil_parse_sockaddr_port error") {
    loop_with_initial_event([]() {
        connect_base<fail>("130.192.16.172", 80,
                           [](Error e, bufferevent *b) {
                               REQUIRE(e);
                               REQUIRE(b == nullptr);
                               break_loop();
                           },
                           3.14);
    });
}

static bufferevent *fail(event_base *, evutil_socket_t, int) { return nullptr; }

TEST_CASE("connect_base deals with bufferevent_socket_new error") {
    bool ok = false;
    try {
        connect_base<::evutil_parse_sockaddr_port, fail>("130.192.16.172", 80,
                                                         nullptr, 3.14);
    } catch (GenericError &) {
        ok = true;
    }
    REQUIRE(ok);
}

static int fail(bufferevent *, const timeval *, const timeval *) { return -1; }

TEST_CASE("connect_base deals with bufferevent_set_timeouts error") {
    bool ok = false;
    try {
        connect_base<::evutil_parse_sockaddr_port, ::bufferevent_socket_new,
                     fail>("130.192.16.172", 80, nullptr, 3.14);
    } catch (GenericError &) {
        ok = true;
    }
    REQUIRE(ok);
}

static int fail(bufferevent *, sockaddr *, int) { return -1; }

TEST_CASE("connect_base deals with bufferevent_socket_connect error") {
    // Note: connectivity not required to run this test
    loop_with_initial_event([]() {
        connect_base<::evutil_parse_sockaddr_port, ::bufferevent_socket_new,
                     bufferevent_set_timeouts, fail>(
            "130.192.16.172", 80,
            [](Error e, bufferevent *b) {
                REQUIRE(e);
                REQUIRE(b == nullptr);
                break_loop();
            },
            3.14);
    });
}

static void success(std::string, int, Callback<Error, Var<Transport>> cb,
                    Settings, Var<Logger> logger, Var<Reactor>) {
    cb(NoError(), Var<Transport>(new Emitter(logger)));
}

TEST_CASE("net::connect_many() correctly handles net::connect() success") {
    Var<ConnectManyCtx> ctx =
        connect_many_make("www.google.com", 80, 3,
                          [](Error err, std::vector<Var<Transport>> conns) {
                              REQUIRE(!err);
                              REQUIRE(conns.size() == 3);
                          },
                          {}, Logger::global(), Reactor::global());
    connect_many_impl<success>(ctx);
}

static void fail(std::string, int, Callback<Error, Var<Transport>> cb, Settings,
                 Var<Logger>, Var<Reactor>) {
    cb(GenericError(), Var<Transport>(nullptr));
}

TEST_CASE("net::connect_many() correctly handles net::connect() failure") {
    Var<ConnectManyCtx> ctx =
        connect_many_make("www.google.com", 80, 3,
                          [](Error err, std::vector<Var<Transport>> conns) {
                              REQUIRE(err);
                              REQUIRE(conns.size() == 0);
                          },
                          {}, Logger::global(), Reactor::global());
    connect_many_impl<fail>(ctx);
}

/*
 _       _                       _   _
(_)_ __ | |_ ___  __ _ _ __ __ _| |_(_) ___  _ __
| | '_ \| __/ _ \/ _` | '__/ _` | __| |/ _ \| '_ \
| | | | | ||  __/ (_| | | | (_| | |_| | (_) | | | |
|_|_| |_|\__\___|\__, |_|  \__,_|\__|_|\___/|_| |_|
                 |___/
*/

TEST_CASE("connect_base works with ipv4") {
    loop_with_initial_event_and_connectivity([]() {
        connect_base("130.192.16.172", 80,
                     [](Error err, bufferevent *bev) {
                         REQUIRE(!err);
                         REQUIRE(bev);
                         ::bufferevent_free(bev);
                         break_loop();
                     },
                     3.14);
    });
}

static bool check_error(Error err) {
    return err == NetworkError() or err == TimeoutError();
}

TEST_CASE("connect_base works with ipv4 and closed port") {
    loop_with_initial_event_and_connectivity([]() {
        connect_base("130.192.16.172", 81,
                     [](Error err, bufferevent *bev) {
                         REQUIRE(check_error(err));
                         REQUIRE(bev == nullptr);
                         break_loop();
                     },
                     3.14);
    });
}

TEST_CASE("connect_base works with ipv4 and timeout") {
    loop_with_initial_event_and_connectivity([]() {
        connect_base("130.192.16.172", 80,
                     [](Error err, bufferevent *bev) {
                         REQUIRE(err == TimeoutError());
                         REQUIRE(bev == nullptr);
                         break_loop();
                     },
                     0.00001);
    });
}

TEST_CASE("connect_base works with ipv6") {
    loop_with_initial_event_and_connectivity([]() {
        connect_base("2a00:1450:4001:801::1004", 80,
                     [](Error err, bufferevent *bev) {
                         if (err) {
                             REQUIRE(err);
                             REQUIRE(bev == nullptr);
                         } else {
                             REQUIRE(!err);
                             REQUIRE(bev);
                             ::bufferevent_free(bev);
                         }
                         break_loop();
                     },
                     3.14);
    });
}

TEST_CASE("connect_first_of works with empty vector") {
    loop_with_initial_event_and_connectivity([]() {
        connect_first_of({}, 80,
                         [](std::vector<Error> errors, bufferevent *bev) {
                             REQUIRE(errors.size() == 0);
                             REQUIRE(bev == nullptr);
                             break_loop();
                         },
                         {{"net/timeout", 3.14}});
    });
}

TEST_CASE("connect_first_of works when all connect fail") {
    loop_with_initial_event_and_connectivity([]() {
        connect_first_of(
            {
                "130.192.16.172", "130.192.16.172", "130.192.16.172",
            },
            80,
            [](std::vector<Error> errors, bufferevent *bev) {
                REQUIRE(errors.size() == 3);
                for (Error err : errors) {
                    REQUIRE(err == TimeoutError());
                }
                REQUIRE(bev == nullptr);
                break_loop();
            },
            {{"net/timeout", 0.00001}});
    });
}

TEST_CASE("connect_first_of works when a connect succeeds") {
    loop_with_initial_event_and_connectivity([]() {
        connect_first_of(
            {
                "130.192.16.172", "130.192.16.172", "130.192.16.172",
            },
            80,
            [](std::vector<Error> errors, bufferevent *bev) {
                REQUIRE(errors.size() == 1);
                for (Error err : errors) {
                    REQUIRE(err == NoError());
                }
                REQUIRE(bev);
                ::bufferevent_free(bev);
                break_loop();
            },
            {{"net/timeout", 3.14}});
    });
}

TEST_CASE("resolve_hostname works with IPv4 address") {
    loop_with_initial_event_and_connectivity([]() {
        std::string hostname = "130.192.16.172";
        resolve_hostname(hostname, [hostname](ResolveHostnameResult r) {
            REQUIRE(r.inet_pton_ipv4);
            REQUIRE(r.addresses.size() == 1);
            REQUIRE(r.addresses[0] == hostname);
            break_loop();
        });
    });
}

TEST_CASE("resolve_hostname works with IPv6 address") {
    loop_with_initial_event_and_connectivity([]() {
        std::string hostname = "2a00:1450:400d:807::200e";
        resolve_hostname(hostname, [hostname](ResolveHostnameResult r) {
            REQUIRE(r.inet_pton_ipv6);
            REQUIRE(r.addresses.size() == 1);
            REQUIRE(r.addresses[0] == hostname);
            break_loop();
        });
    });
}

TEST_CASE("resolve_hostname works with domain") {
    loop_with_initial_event_and_connectivity([]() {
        resolve_hostname("google.com", [](ResolveHostnameResult r) {
            REQUIRE(not r.inet_pton_ipv4);
            REQUIRE(not r.inet_pton_ipv6);
            REQUIRE(not r.ipv4_err);
            REQUIRE(not r.ipv6_err);
            // At least one IPv4 and one IPv6 addresses
            REQUIRE(r.addresses.size() > 1);
            break_loop();
        });
    });
}

TEST_CASE("stress resolve_hostname with invalid address and domain") {
    loop_with_initial_event_and_connectivity([]() {
        // Pass input that is neither invalid IPvX nor valid domain
        resolve_hostname("192.1688.antani", [](ResolveHostnameResult r) {
            REQUIRE(not r.inet_pton_ipv4);
            REQUIRE(not r.inet_pton_ipv6);
            REQUIRE(r.ipv4_err);
            REQUIRE(r.ipv6_err);
            REQUIRE(r.addresses.size() == 0);
            break_loop();
        });
    });
}

TEST_CASE("connect() works with valid IPv4") {
    loop_with_initial_event_and_connectivity([]() {
        connect_logic("www.google.com", 80, [](Error e, Var<ConnectResult> r) {
            REQUIRE(!e);
            REQUIRE(r->connected_bev != nullptr);
            ::bufferevent_free(r->connected_bev);
            break_loop();
        });
    });
}

TEST_CASE("connect() fails when port is closed or filtered") {
    loop_with_initial_event_and_connectivity([]() {
        connect_logic("www.google.com", 81, [](Error e, Var<ConnectResult> r) {
            REQUIRE(e);
            REQUIRE(r->connected_bev == nullptr);
            break_loop();
        });
    });
}

TEST_CASE("connect() fails when setting an invalid dns") {
    loop_with_initial_event_and_connectivity([]() {
        connect_logic("www.google.com", 80,
                      [](Error e, Var<ConnectResult> r) {
                          REQUIRE(e);
                          REQUIRE(r->connected_bev == nullptr);
                          break_loop();
                      },
                      {{"dns/nameserver", "8.8.8.1"},
                       {"dns/timeout", 0.001},
                       {"dns/attempts", 1}});
    });
}

TEST_CASE("net::connect_many() works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        connect_many("www.google.com", 80, 3,
                     [](Error error, std::vector<Var<Transport>> conns) {
                         REQUIRE(!error);
                         REQUIRE(conns.size() == 3);
                         Var<int> n(new int(0));
                         for (auto conn : conns) {
                             REQUIRE(static_cast<bool>(conn));
                             conn->close([n]() {
                                 if (++(*n) >= 3) {
                                     break_loop();
                                 }
                             });
                         }
                     });
    });
}

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
                [](Error error, Var<Transport> ) {
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
                [](Error error, Var<Transport>) {
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
