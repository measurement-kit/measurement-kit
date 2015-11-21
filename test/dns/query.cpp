// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/dns.hpp>
#include <measurement_kit/common.hpp>

#include "src/dns/query.hpp"

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

//
// TODO: make sure that Query() correctly handle `dnsb` and `libs`.
//
// Not urgent because already guaranteed by other tests.
//

// Now testing QueryImpl()

TEST_CASE("Query deals with failing evdns_base_resolve_ipv4") {
    Libs libs;

    libs.evdns_base_resolve_ipv4 =
        [](evdns_base *, const char *, int, evdns_callback_type, void *) {
            return (evdns_request *)NULL;
        };

    Query("IN", "A", "www.google.com", [](Error e, Response) {
        REQUIRE(e == ResolverError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query deals with failing evdns_base_resolve_ipv6") {
    Libs libs;

    libs.evdns_base_resolve_ipv6 =
        [](evdns_base *, const char *, int, evdns_callback_type, void *) {
            return (evdns_request *)NULL;
        };

    Query("IN", "AAAA", "github.com", [](Error e, Response) {
        REQUIRE(e == ResolverError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query deals with failing evdns_base_resolve_reverse") {
    Libs libs;

    libs.evdns_base_resolve_reverse =
        [](evdns_base *, const struct in_addr *, int, evdns_callback_type,
           void *) { return (evdns_request *)NULL; };

    Query("IN", "REVERSE_A", "8.8.8.8", [](Error e, Response) {
        REQUIRE(e == ResolverError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query deals with failing evdns_base_resolve_reverse_ipv6") {
    Libs libs;

    libs.evdns_base_resolve_reverse_ipv6 =
        [](evdns_base *, const struct in6_addr *, int, evdns_callback_type,
           void *) { return (evdns_request *)NULL; };

    Query("IN", "REVERSE_AAAA", "::1", [](Error e, Response) {
        REQUIRE(e == ResolverError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query deals with inet_pton returning 0") {
    Libs libs;

    libs.inet_pton = [](int, const char *, void *) { return 0; };

    Query("IN", "REVERSE_A", "8.8.8.8", [](Error e, Response) {
        REQUIRE(e == InvalidIPv4AddressError());
    }, Logger::global(), NULL, &libs);

    Query("IN", "REVERSE_AAAA", "::1", [](Error e, Response) {
        REQUIRE(e == InvalidIPv6AddressError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query deals with inet_pton returning -1") {
    Libs libs;

    libs.inet_pton = [](int, const char *, void *) { return -1; };

    Query("IN", "REVERSE_A", "8.8.8.8", [](Error e, Response) {
        REQUIRE(e == InvalidIPv4AddressError());
    }, Logger::global(), NULL, &libs);

    Query("IN", "REVERSE_AAAA", "::1", [](Error e, Response) {
        REQUIRE(e == InvalidIPv6AddressError());
    }, Logger::global(), NULL, &libs);
}

TEST_CASE("Query raises if the query is unsupported") {
    Query("IN", "MX", "www.neubot.org",
          [](Error e, Response) { REQUIRE(e == UnsupportedTypeError()); });
}

TEST_CASE("Query raises if the class is unsupported") {
    Query("CS", "A", "www.neubot.org",
          [](Error e, Response) { REQUIRE(e == UnsupportedClassError()); });
}

TEST_CASE("Query deals with invalid PTR name") {
    // This should be enough to see the failure, more tests for the
    // parser for PTR addresses are in test/common/utils.cpp
    Query("IN", "PTR", "xx",
          [](Error e, Response) { REQUIRE(e == InvalidNameForPTRError()); });
}

//
// Now testing cancel().
//
// We have two cases: when the request is pending and when the request
// has already returned a value.
//

TEST_CASE("Query::cancel() is idempotent") {

    //
    // Here we only want to see that multiple cancel()s followed
    // by the object being destroyed cause no harm.
    //
    // This is similar to the following test, because in both tests
    // we check that we can get rid of a pending request.
    //

    auto r1 = Query("IN", "A", "www.neubot.org", [&](Error, Response) {
        // nothing
    });

    r1.cancel();
    r1.cancel();
    r1.cancel();
}

TEST_CASE("Query::cancel() is safe when a request is pending") {

    //
    // Note: this test can be run both when the network is down and when
    // the network is up. In both cases, the external Query is deleted
    // before any event happens, and the QueryImpl waits for some
    // event to happen. When the network is down, the event is the timeout
    // of evdns; when the network is up, the event is the DNS reply.
    //

    auto bad_code = false;
    auto called = false;

    Libs libs;
    libs.evdns_reply_hook = [&](int code, char, int, int, void *, void *) {
        called = true;
        if (code != DNS_ERR_NONE && code != DNS_ERR_TIMEOUT) {
            bad_code = true;
        }
        //
        // We need to unblock the loop here, because this test deletes
        // the Query immediately, so we don't have a callback where
        // to report that we are done and we need to break the loop.
        //
        measurement_kit::break_loop();
    };

    auto failed = false;
    {
        auto r1 = Query("IN", "A", "www.neubot.org", [&](Error, Response) {
            //
            // This callback should not be invoked, because QueryImpl
            // should honor its `cancelled` field and therefore should delete
            // itself rather than calling the callback.
            //
            failed = true;
            measurement_kit::break_loop();
        }, Logger::global(), NULL, &libs);

    } // This kills Query, but not the underlying QueryImpl

    measurement_kit::loop();
    REQUIRE(!failed);
    REQUIRE(!bad_code);
    REQUIRE(called);
}

//
// TODO: test the case when we receive the message and then free the request.
//
// Not urgent, because it's tested by lots of other tests.
//

struct TransparentQuery : public Query {
    using Query::Query;

    Var<bool> get_cancelled_() { return cancelled; }
};

TEST_CASE("Move semantic works for request") {

    SECTION("Move assignment") {

        TransparentQuery r1;

        REQUIRE_THROWS(*r1.get_cancelled_());

        TransparentQuery r2{"IN", "A", "www.neubot.org", [](Error, Response) {
            /* nothing */
        }};
        REQUIRE(*r2.get_cancelled_() == false);

        r1 = std::move(r2); /* Move assignment */

        REQUIRE(*r1.get_cancelled_() == false);

        REQUIRE_THROWS(*r2.get_cancelled_());
    }

    SECTION("Move constructor") {
        TransparentQuery r2{"IN", "A", "www.neubot.org", [](Error, Response) {
            /* nothing */
        }};
        REQUIRE(*r2.get_cancelled_() == false);

        [](TransparentQuery r1) {
            REQUIRE(*r1.get_cancelled_() == false);

        }(std::move(r2)); /* Move constructor */

        REQUIRE_THROWS(*r2.get_cancelled_());
    }
}

//
// Integration (or regress?) tests for Query.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The system resolver works as expected") {

    //
    // Note: this test also makes sure that we get sensible
    // response fields from the system resolver.
    //

    if (CheckConnectivity::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        measurement_kit::break_loop();
    });

    auto r1 =
        Query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "130.192.16.172");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            measurement_kit::break_loop();
        });
    measurement_kit::loop();

    auto r2 = Query(
        "IN", "REVERSE_A", "130.192.16.172", [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            measurement_kit::break_loop();
        });
    measurement_kit::loop();

    auto r2bis =
        Query("IN", "PTR", "172.16.192.130.in-addr.arpa.",
              [&](Error e, Response response) {
                  REQUIRE(!e);
                  REQUIRE(response.get_reply_authoritative() == "unknown");
                  REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                  REQUIRE(response.get_results().size() == 1);
                  REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
                  REQUIRE(response.get_rtt() > 0.0);
                  REQUIRE(response.get_ttl() > 0);
                  measurement_kit::break_loop();
              });
    measurement_kit::loop();

    auto r3 = Query(
        "IN", "AAAA", "ooni.torproject.org", [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() > 0);
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            auto found = false;
            for (auto address : response.get_results()) {
                if (address == "2001:41b8:202:deb:213:21ff:fe20:1426") {
                    found = true;
                }
            }
            REQUIRE(found);
            measurement_kit::break_loop();
        });
    measurement_kit::loop();

    auto r4 = Query(
        "IN", "REVERSE_AAAA", "2001:41b8:202:deb:213:21ff:fe20:1426",
        [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "listera.torproject.org");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            measurement_kit::break_loop();
        });
    measurement_kit::loop();

    auto r4bis = Query(
        "IN", "PTR", "6.2.4.1.0.2.e.f.f.f.1.2.3.1.2.0.b.e.d.0.2.0.2.0.8.b.1.4."
                     "1.0.0.2.ip6.arpa.",
        [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "listera.torproject.org");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            measurement_kit::break_loop();
        });
    measurement_kit::loop();

    REQUIRE(!failed);
}

class SafeToDeleteQueryInItsOwnCallback {
    Query request;

  public:
    SafeToDeleteQueryInItsOwnCallback() {
        request = Query("IN", "A", "nexa.polito.it", [this](Error, Response) {
            // This assignment should trigger the original request's destructor
            request =
                Query("IN", "AAAA", "nexa.polito.it", [this](Error, Response) {
                    measurement_kit::break_loop();
                });
        });
    }
};

TEST_CASE("It is safe to clear a request in its own callback") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    auto d = SafeToDeleteQueryInItsOwnCallback();
    auto called = 0;
    DelayedCall watchdog(5.0, [&called]() {
        ++called;
        measurement_kit::break_loop();
    });
    measurement_kit::loop();
    REQUIRE(called == 0);
}
