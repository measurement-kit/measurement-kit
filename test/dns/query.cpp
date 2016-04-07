// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>

#include "src/dns/query.hpp"
#include "src/common/check_connectivity.hpp"
#include "src/common/delayed_call.hpp"
#include "src/common/libs_impl.hpp"

using namespace mk;
using namespace mk::dns;

// Now testing query()
inline evdns_request * null_resolver (evdns_base *, const char *, int , evdns_callback_type, void *) {
    return (evdns_request *)NULL;
}
inline evdns_request * null_resolver_reverse (evdns_base *, const struct in_addr *, int , evdns_callback_type, void *) {
    return (evdns_request *)NULL;
}
inline evdns_request * null_resolver_reverse (evdns_base *, const struct in6_addr *in, int , evdns_callback_type, void *) {
    return (evdns_request *)NULL;
}
inline int null_inet_pton (int, const char *, void *) {
    return 0;
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_ipv4") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    query_debug<::evdns_base_free, null_resolver>("IN", "A", "www.google.com",
            [](Error e, Message) { REQUIRE(e == ResolverError()); }, {}
            , get_global_poller());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_ipv6") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    query_debug<::evdns_base_free, ::evdns_base_resolve_ipv4, null_resolver>("IN", "AAAA", "github.com",
            [](Error e, Message) { REQUIRE(e == ResolverError()); }, {},
            get_global_poller());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    query_debug <::evdns_base_free, ::evdns_base_resolve_ipv4, 
                 ::evdns_base_resolve_ipv6, null_resolver_reverse> 
                    ("IN", "REVERSE_A", "8.8.8.8", [](Error e, Message) {
                REQUIRE(e == ResolverError());
                }, {}, get_global_poller());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse_ipv6") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    query_debug <::evdns_base_free, ::evdns_base_resolve_ipv4, 
                 ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                 null_resolver_reverse> 
                    ("IN", "REVERSE_AAAA", "::1", [](Error e, Message) {
                    REQUIRE(e == ResolverError());
                }, {}, get_global_poller());
}

TEST_CASE("dns::query deals with inet_pton returning 0") {
    query_debug <::evdns_base_free, ::evdns_base_resolve_ipv4,
                 ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                 ::evdns_base_resolve_reverse_ipv6, null_inet_pton> 
                ("IN", "REVERSE_A", "8.8.8.8", [](Error e, Message) {
         REQUIRE(e == InvalidIPv4AddressError());
    }, {}, get_global_poller());

    query_debug <::evdns_base_free, ::evdns_base_resolve_ipv4,
                 ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                 ::evdns_base_resolve_reverse_ipv6, null_inet_pton> 
                ("IN", "REVERSE_AAAA", "::1", [](Error e, Message) {
         REQUIRE(e == InvalidIPv6AddressError());
    }, {}, get_global_poller());
}

TEST_CASE("dns::query deals with inet_pton returning -1") {
    libs.inet_pton = [](int, const char *, void *) { return -1; };

    // TODO refactor this to use Templates
    /* query("IN", "REVERSE_A", "8.8.8.8", [](Error e, Response) { */
    /*     REQUIRE(e == InvalidIPv4AddressError()); */
    /* }, Logger::global(), NULL, &libs); */

    /* query("IN", "REVERSE_AAAA", "::1", [](Error e, Response) { */
    /*     REQUIRE(e == InvalidIPv6AddressError()); */
    /* }, Logger::global(), NULL, &libs); */
}

TEST_CASE("dns::query raises if the query is unsupported") {
    query("IN", "MX", "www.neubot.org",
            [](Error e, Message) { REQUIRE(e == UnsupportedTypeError()); });
}

TEST_CASE("dns::query raises if the class is unsupported") {
    query("CS", "A", "www.neubot.org",
            [](Error e, Message) { REQUIRE(e == UnsupportedClassError()); });
}

TEST_CASE("dns::query deals with invalid PTR name") {
    // This should be enough to see the failure, more tests for the
    // parser for PTR addresses are in test/common/utils.cpp
    query("IN", "PTR", "xx",
            [](Error e, Message) { REQUIRE(e == InvalidNameForPTRError()); });
}

// Integration (or regress?) tests for dns::query.
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

    query("IN", "A", "www.neubot.org", [&](Error e, Message message) {
        REQUIRE(!e);
        REQUIRE(message.error_code == DNS_ERR_NONE);
        REQUIRE(message.answers.size() == 1);
        REQUIRE(message.answers[0].ipv4 == "130.192.16.172");
        REQUIRE(message.rtt > 0.0);
        REQUIRE(message.answers[0].ttl > 0);
        mk::break_loop();
    });
    mk::loop();

    query("IN", "REVERSE_A", "130.192.16.172", [&](Error e, Message message) {
        REQUIRE(!e);
        REQUIRE(message.error_code == DNS_ERR_NONE);
        REQUIRE(message.answers.size() == 1);
        REQUIRE(message.answers[0].hostname == "server-nexa.polito.it");
        REQUIRE(message.rtt > 0.0);
        REQUIRE(message.answers[0].ttl > 0);
        mk::break_loop();
    });
    mk::loop();

    query("IN", "PTR", "172.16.192.130.in-addr.arpa.",
            [&](Error e, Message message) {
                REQUIRE(!e);
                REQUIRE(message.error_code == DNS_ERR_NONE);
                REQUIRE(message.answers.size() == 1);
                REQUIRE(message.answers[0].hostname == "server-nexa.polito.it");
                REQUIRE(message.rtt > 0.0);
                REQUIRE(message.answers[0].ttl > 0);
                mk::break_loop();
            });
    mk::loop();

    query("IN", "AAAA", "ooni.torproject.org", [&](Error e, Message message) {
        REQUIRE(!e);
        REQUIRE(message.error_code == DNS_ERR_NONE);
        REQUIRE(message.answers.size() > 0);
        REQUIRE(message.rtt > 0.0);
        REQUIRE(message.answers[0].ttl > 0);
        auto found = false;
        for (auto answer : message.answers) {
            if (answer.ipv6 == "2001:41b8:202:deb:213:21ff:fe20:1426") {
                found = true;
            }
        }
        REQUIRE(found);
        mk::break_loop();
    });
    mk::loop();

    query("IN", "REVERSE_AAAA", "2001:41b8:202:deb:213:21ff:fe20:1426",
            [&](Error e, Message message) {
                REQUIRE(!e);
                REQUIRE(message.error_code == DNS_ERR_NONE);
                REQUIRE(message.answers.size() == 1);
                REQUIRE(message.answers[0].hostname ==
                        "listera.torproject.org");
                REQUIRE(message.rtt > 0.0);
                REQUIRE(message.answers[0].ttl > 0);
                mk::break_loop();
            });
    mk::loop();

    query("IN", "PTR",
            "6.2.4.1.0.2.e.f.f.f.1.2.3.1.2.0.b.e.d.0.2.0.2.0.8.b.1.4."
            "1.0.0.2.ip6.arpa.",
            [&](Error e, Message message) {
                REQUIRE(!e);
                REQUIRE(message.error_code == DNS_ERR_NONE);
                REQUIRE(message.answers.size() == 1);
                REQUIRE(message.answers[0].hostname ==
                        "listera.torproject.org");
                REQUIRE(message.rtt > 0.0);
                REQUIRE(message.answers[0].ttl > 0);
                mk::break_loop();
            });
    mk::loop();
}
