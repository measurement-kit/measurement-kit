// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/net/connect_impl.hpp"
#include "src/libmeasurement_kit/net/error.hpp"
#include "src/libmeasurement_kit/net/emitter.hpp"

#include <event2/bufferevent.h>

#include <iostream>

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

static Error fail(std::string, uint16_t, sockaddr_storage *, socklen_t *) {
    return ValueError();
}

TEST_CASE("connect_base deals with evutil_parse_sockaddr_port error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base<fail>("130.192.16.172", 80, 3.14, reactor, Logger::global(),
                           [=](Error e, bufferevent *b, double) {
                               REQUIRE(e);
                               REQUIRE(b == nullptr);
                               reactor->stop();
                           });
    });
}

static bufferevent *fail(event_base *, evutil_socket_t, int) { return nullptr; }

TEST_CASE("connect_base deals with bufferevent_socket_new error") {
    bool ok = false;
    try {
        connect_base<make_sockaddr, fail>("130.192.16.172", 80, 3.14,
                Reactor::global(), Logger::global(), nullptr);
    } catch (GenericError &) {
        ok = true;
    }
    REQUIRE(ok);
}

static int fail(bufferevent *, const timeval *, const timeval *) { return -1; }

TEST_CASE("connect_base deals with bufferevent_set_timeouts error") {
    bool ok = false;
    try {
        connect_base<make_sockaddr, ::bufferevent_socket_new,
                     fail>("130.192.16.172", 80, 3.14, Reactor::global(),
                           Logger::global(), nullptr);
    } catch (GenericError &) {
        ok = true;
    }
    REQUIRE(ok);
}

/*
 * The first callback is for libevent 2.0 and the second is for libevent
 * 2.1, where the API has changed. The C++ compiler will select the function
 * that matches the expected prototype, according to SFINAE principles.
 */
class Fail {
  public:
    static int fail(bufferevent *, sockaddr *, int) { return -1; }
    static int fail(bufferevent *, const sockaddr *, int) { return -1; }
};

TEST_CASE("connect_base deals with bufferevent_socket_connect error") {
    // Note: connectivity not required to run this test
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base<make_sockaddr, ::bufferevent_socket_new,
                     bufferevent_set_timeouts, Fail::fail>(
            "130.192.16.172", 80, 3.14, reactor, Logger::global(),
            [=](Error e, bufferevent *b, double) {
                REQUIRE(e);
                REQUIRE(b == nullptr);
                reactor->stop();
            });
    });
}

static void success(std::string, int, Callback<Error, SharedPtr<Transport>> cb,
                    Settings, SharedPtr<Reactor> r, SharedPtr<Logger> logger) {
    cb(NoError(), SharedPtr<Transport>(new Emitter(r, logger)));
}

TEST_CASE("net::connect_many() correctly handles net::connect() success") {
    SharedPtr<Reactor> reactor = Reactor::make();
    SharedPtr<ConnectManyCtx> ctx =
        connect_many_make("www.google.com", 80, 3,
                          [](Error err, std::vector<SharedPtr<Transport>> conns) {
                              REQUIRE(!err);
                              REQUIRE(conns.size() == 3);
                          },
                          {}, reactor, Logger::global());
    connect_many_impl<success>(ctx);
}

static void fail(std::string, int, Callback<Error, SharedPtr<Transport>> cb, Settings,
                 SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(GenericError(), SharedPtr<Transport>(nullptr));
}

TEST_CASE("net::connect_many() correctly handles net::connect() failure") {
    SharedPtr<Reactor> reactor = Reactor::make();
    SharedPtr<ConnectManyCtx> ctx =
        connect_many_make("www.google.com", 80, 3,
                          [](Error err, std::vector<SharedPtr<Transport>> conns) {
                              REQUIRE(err);
                              REQUIRE(conns.size() == 1);
                          },
                          {}, reactor, Logger::global());
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

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("connect_base works with ipv4") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base("130.192.16.172", 80, 3.14, reactor, Logger::global(),
                     [=](Error err, bufferevent *bev, double) {
                         REQUIRE(!err);
                         REQUIRE(bev);
                         ::bufferevent_free(bev);
                         reactor->stop();
                     });
    });
}

static bool check_error(Error err) {
    return err == ConnectionRefusedError() or err == TimeoutError();
}

TEST_CASE("connect_base works with ipv4 and closed port") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base("130.192.16.172", 81, 3.14, reactor, Logger::global(),
                     [=](Error err, bufferevent *bev, double) {
                         REQUIRE(check_error(err));
                         REQUIRE(bev == nullptr);
                         reactor->stop();
                     });
    });
}

TEST_CASE("connect_base works with ipv4 and timeout") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base("130.192.16.172", 80, 0.00001, reactor, Logger::global(),
                     [=](Error err, bufferevent *bev, double) {
                         REQUIRE(err == TimeoutError());
                         REQUIRE(bev == nullptr);
                         reactor->stop();
                     });
    });
}

TEST_CASE("connect_base works with ipv6") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_base("2a00:1450:4001:801::1004", 80, 3.14, reactor, Logger::global(),
                     [=](Error err, bufferevent *bev, double) {
                         /* Coverage note: depending on whether IPv6
                            works or not here we're going to see either
                            branch covered. */
                         if (err) {
                             REQUIRE(err);
                             REQUIRE(bev == nullptr);
                         } else {
                             REQUIRE(!err);
                             REQUIRE(bev);
                             ::bufferevent_free(bev);
                         }
                         reactor->stop();
                     });
    });
}

TEST_CASE("connect_first_of works with empty vector") {
    SharedPtr<Reactor> reactor = Reactor::make();
    SharedPtr<ConnectResult> result(new ConnectResult);
    reactor->run_with_initial_event([=]() {
        connect_first_of(result, 80,
                         [=](std::vector<Error> errors, bufferevent *bev) {
                             REQUIRE(errors.size() == 0);
                             REQUIRE(bev == nullptr);
                             reactor->stop();
                         },
                         {{"net/timeout", 3.14}}, reactor);
    });
}

TEST_CASE("connect_first_of works when all connect fail") {
    SharedPtr<Reactor> reactor = Reactor::make();
    SharedPtr<ConnectResult> result(new ConnectResult);
    result->resolve_result.addresses = {
        "130.192.16.172", "130.192.16.172", "130.192.16.172",
    };
    reactor->run_with_initial_event([&result, reactor]() {
        connect_first_of(
            result,
            80,
            [=](std::vector<Error> errors, bufferevent *bev) {
                REQUIRE(errors.size() == 3);
                for (Error err : errors) {
                    REQUIRE(err == TimeoutError());
                }
                REQUIRE(bev == nullptr);
                reactor->stop();
            },
            {{"net/timeout", 0.00001}}, reactor);
    });
}

TEST_CASE("connect_first_of works when a connect succeeds") {
    SharedPtr<Reactor> reactor = Reactor::make();
    SharedPtr<ConnectResult> result(new ConnectResult);
    result->resolve_result.addresses = {
        "130.192.16.172", "130.192.16.172", "130.192.16.172",
    };
    reactor->run_with_initial_event([&result, reactor]() {
        connect_first_of(
            result,
            80,
            [=](std::vector<Error> errors, bufferevent *bev) {
                REQUIRE(errors.size() == 1);
                for (Error err : errors) {
                    REQUIRE(err == NoError());
                }
                REQUIRE(bev);
                ::bufferevent_free(bev);
                reactor->stop();
            },
            {{"net/timeout", 3.14}}, reactor);
    });
}

TEST_CASE("connect() works with valid IPv4") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_logic("www.google.com", 80, [=](Error e, SharedPtr<ConnectResult> r) {
            REQUIRE(!e);
            REQUIRE(r->connected_bev != nullptr);
            ::bufferevent_free(r->connected_bev);
            reactor->stop();
        }, {}, reactor);
    });
}

TEST_CASE("connect() fails when port is closed or filtered") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_logic("www.google.com", 81, [=](Error e, SharedPtr<ConnectResult> r) {
            REQUIRE(e);
            REQUIRE(r->connected_bev == nullptr);
            reactor->stop();
        }, {{"net/timeout", 1.0}}, reactor);
    });
}

TEST_CASE("connect() fails when setting an invalid dns") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_logic("www.google.com", 80,
                      [=](Error e, SharedPtr<ConnectResult> r) {
                          REQUIRE(e);
                          REQUIRE(r->connected_bev == nullptr);
                          reactor->stop();
                      },
                      {{"dns/nameserver", "8.8.8.1"},
                       {"dns/timeout", 0.001},
                       {"dns/engine", "libevent"},
                       {"dns/attempts", 1}},
                      reactor);
    });
}

TEST_CASE("net::connect_many() works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect_many("www.google.com", 80, 3,
                     [=](Error error, std::vector<SharedPtr<Transport>> conns) {
                         REQUIRE(!error);
                         REQUIRE(conns.size() == 3);
                         SharedPtr<int> n(new int(0));
                         for (auto conn : conns) {
                             REQUIRE(static_cast<bool>(conn));
                             conn->close([=]() {
                                 if (++(*n) >= 3) {
                                     reactor->stop();
                                 }
                             });
                         }
                     }, {}, reactor);
    });
}

TEST_CASE("net::connect() works with a custom reactor") {
    // Note: this is how Portolan uses measurement-kit
    SharedPtr<Reactor> reactor = Reactor::make();
    auto ok = false;
    reactor->run_with_initial_event([&]() {
        connect("nexa.polito.it", 22,
                [&](Error error, SharedPtr<Transport> txp) {
                    if (error) {
                        reactor->stop();
                        return;
                    }
                    txp->close([&]() { reactor->stop(); });
                    ok = true;
                },
                {}, reactor, Logger::global());
    });
    REQUIRE(ok);
}

TEST_CASE("net::connect() can connect to open port") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("www.kernel.org", 80, [=](Error error, SharedPtr<Transport> txp) {
            REQUIRE(!error);
            auto resolve_result = txp->dns_result();
            REQUIRE(!resolve_result.inet_pton_ipv4);
            REQUIRE(!resolve_result.inet_pton_ipv6);
            REQUIRE(resolve_result.addresses.size() > 0);
            txp->close([=]() { reactor->stop(); });
        }, {}, reactor);
    });
}

TEST_CASE("net::connect() can connect to ssl port") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("nexa.polito.it", 443,
                [=](Error error, SharedPtr<Transport> txp) {
                    REQUIRE(!error);
                    auto resolve_result = txp->dns_result();
                    REQUIRE(!resolve_result.inet_pton_ipv4);
                    REQUIRE(!resolve_result.inet_pton_ipv6);
                    REQUIRE(resolve_result.addresses.size() > 0);
                    txp->close([=]() { reactor->stop(); });
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/saved_ca_bundle.pem"}},
                reactor);
    });
}

TEST_CASE("net::connect() ssl fails when presented an expired certificate") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("expired.badssl.com", 443,
                [=](Error error, SharedPtr<Transport> ) {
                    REQUIRE(error);
                    reactor->stop();
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/saved_ca_bundle.pem"}},
                reactor);
    });
}

TEST_CASE("net::connect() ssl fails when presented a certificate with the "
          "wrong hostname") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("wrong.host.badssl.com", 443,
                [=](Error error, SharedPtr<Transport>) {
                    REQUIRE(error);
                    reactor->stop();
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/saved_ca_bundle.pem"}},
                reactor);
    });
}

TEST_CASE("net::connect() ssl works when using SNI") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("sha256.badssl.com", 443,
                [=](Error error, SharedPtr<Transport> txp) {
                    REQUIRE(!error);
                    txp->close([=]() { reactor->stop(); });
                },
                {{"net/ssl", true},
                 {"net/ca_bundle_path", "test/fixtures/saved_ca_bundle.pem"}},
                reactor);
    });
}

TEST_CASE("net::connect() works in case of error") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("nexa.polito.it", 81,
                [=](Error error, SharedPtr<Transport> txp) {
                    REQUIRE(error);
                    auto resolve_result = txp->dns_result();
                    REQUIRE(!resolve_result.inet_pton_ipv4);
                    REQUIRE(!resolve_result.inet_pton_ipv6);
                    REQUIRE(resolve_result.addresses.size() > 0);
                    reactor->stop();
                },
                {{"net/timeout", 5.0}},
                reactor);
    });
}

#endif
