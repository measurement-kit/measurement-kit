// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/net/connect.{c,h}pp
//

#define CATCH_CONFIG_MAIN

#include "src/net/connect.hpp"
#include "src/common/check_connectivity.hpp"
#include "src/ext/Catch/single_include/catch.hpp"
#include <event2/bufferevent.h>
#include <iostream>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/net/error.hpp>
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
        connect_base<::evutil_parse_sockaddr_port, fail>(
                "130.192.16.172", 80, nullptr, 3.14);
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
    loop_with_initial_event([]() {
        connect_base<::evutil_parse_sockaddr_port, ::bufferevent_socket_new,
                bufferevent_set_timeouts, fail>("130.192.16.172", 80,
                [](Error e, bufferevent *b) {
                    REQUIRE(e);
                    REQUIRE(b == nullptr);
                    break_loop();
                },
                3.14);
    });
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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

static bool check_error (Error err) {
    return err == NetworkError() or err == TimeoutError();
}

TEST_CASE("connect_base works with ipv4 and closed port") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect_first_of({}, 80,
                [](std::vector<Error> errors, bufferevent *bev) {
                    REQUIRE(errors.size() == 0);
                    REQUIRE(bev == nullptr);
                    break_loop();
                },
                3.14);
    });
}

TEST_CASE("connect_first_of works when all connect fail") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
                0.00001);
    });
}

TEST_CASE("connect_first_of works when a connect succeeds") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
                3.14);
    });
}

TEST_CASE("resolve_hostname works with IPv4 address") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
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
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("www.google.com", 80, [](Error e, Var<ConnectResult> r) {
            REQUIRE(!e);
            REQUIRE(r->connected_bev != nullptr);
            ::bufferevent_free(r->connected_bev);
            break_loop();
        });
    });
}

TEST_CASE("connect() fails when port is closed or filtered") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect("www.google.com", 81, [](Error e, Var<ConnectResult> r) {
            REQUIRE(e);
            REQUIRE(r->connected_bev == nullptr);
            break_loop();
        });
    });
}
