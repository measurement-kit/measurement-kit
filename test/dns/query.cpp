// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/dns/query_impl.hpp"
#include <measurement_kit/dns.hpp>

using namespace mk;
using namespace mk::dns;

// Now testing query()
static evdns_request *null_resolver(evdns_base *, const char *, int,
                                    evdns_callback_type, void *) {
    return (evdns_request *)nullptr;
}
static evdns_request *null_resolver_reverse(evdns_base *,
                                            const struct in_addr *, int,
                                            evdns_callback_type, void *) {
    return (evdns_request *)nullptr;
}
static evdns_request *null_resolver_reverse(evdns_base *,
                                            const struct in6_addr *, int,
                                            evdns_callback_type, void *) {
    return (evdns_request *)nullptr;
}
static int null_inet_pton(int, const char *, void *) { return 0; }

static const char *null_inet_ntop(int, const void *, char *, socklen_t) {
    return nullptr;
}

static evdns_base *null_evdns_base_new(event_base *, int) {
    return (evdns_base *)nullptr;
}

static int null_evdns_base_nameserver_ip_add(evdns_base *, const char *) {
    return -1;
}

static int null_evdns_base_set_option_randomize(evdns_base *, const char *,
                                                const char *) {
    return -1;
}

#define BASE_FREE(name)                                                        \
    static bool base_free_##name##_flag = false;                               \
    static void base_free_##name(struct evdns_base *base, int fail_requests) { \
        ::evdns_base_free(base, fail_requests);                                \
        base_free_##name##_flag = true;                                        \
    }

BASE_FREE(evdns_base_nameserver_ip_add)
BASE_FREE(evdns_set_options_attempts)
BASE_FREE(evdns_set_options_attempts_negative)
BASE_FREE(evdns_set_options_timeout)
BASE_FREE(evdns_set_options_randomize)

TEST_CASE("throw error while fails evdns_base_new") {
    REQUIRE_THROWS_AS(
        create_evdns_base<null_evdns_base_new>({}, Reactor::global()),
        std::bad_alloc);
}

TEST_CASE("throw error while fails evdns_base_nameserver_ip_add") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, null_evdns_base_nameserver_ip_add,
                           base_free_evdns_base_nameserver_ip_add>(
            {{"dns/nameserver", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_base_nameserver_ip_add_flag);
}

TEST_CASE("throw error while fails evdns_base_nameserver_ip_add and base_new") {
    REQUIRE_THROWS_AS((create_evdns_base<null_evdns_base_new,
                                         null_evdns_base_nameserver_ip_add>(
                          {{"dns/nameserver", "nexa"}}, Reactor::global())),
                      std::bad_alloc);
}

TEST_CASE("throw error while fails evdns_set_options for attempts") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_ip_add,
                           base_free_evdns_set_options_attempts>(
            {{"dns/attempts", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_attempts_flag);
}

TEST_CASE("throw error while fails evdns_set_options for timeout") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_ip_add,
                           base_free_evdns_set_options_timeout>(
            {{"dns/attempts", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_timeout_flag);
}

TEST_CASE("throw error while fails evdns_set_options for negative attempts") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_ip_add,
                           base_free_evdns_set_options_attempts_negative>(
            {{"dns/attempts", -1}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_attempts_negative_flag);
}

TEST_CASE("throw error while fails evdns_set_options for randomize-case") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_ip_add,
                           base_free_evdns_set_options_randomize,
                           null_evdns_base_set_option_randomize>(
            {{"dns/randomize_case", ""}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_randomize_flag);
}

TEST_CASE("throw error with too many addresses") {
    REQUIRE_THROWS_AS(build_answers_evdns(DNS_ERR_NONE, DNS_IPv4_A,
                                          (INT_MAX / 4) + 2, 20, nullptr),
                      std::runtime_error);
}

TEST_CASE("throw error with ntop conversion error") {
    REQUIRE_THROWS_AS(build_answers_evdns<null_inet_ntop>(
                          DNS_ERR_NONE, DNS_IPv4_A, 1, 20, nullptr),
                      std::runtime_error);
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_ipv4") {
    query_impl<::evdns_base_free, null_resolver>(
        "IN", "A", "www.google.com",
        [](Error e, Message) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_ipv6") {
    query_impl<::evdns_base_free, ::evdns_base_resolve_ipv4, null_resolver>(
        "IN", "AAAA", "github.com",
        [](Error e, Message) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse") {
    query_impl<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, null_resolver_reverse>(
        "IN", "REVERSE_A", "8.8.8.8",
        [](Error e, Message) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse_ipv6") {
    query_impl<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                null_resolver_reverse>(
        "IN", "REVERSE_AAAA", "::1",
        [](Error e, Message) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global());
}

TEST_CASE("dns::query deals with inet_pton returning 0") {
    query_impl<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                ::evdns_base_resolve_reverse_ipv6, null_inet_pton>(
        "IN", "REVERSE_A", "8.8.8.8",
        [](Error e, Message) { REQUIRE(e == InvalidIPv4AddressError()); }, {},
        Reactor::global());

    query_impl<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                ::evdns_base_resolve_reverse_ipv6, null_inet_pton>(
        "IN", "REVERSE_AAAA", "::1",
        [](Error e, Message) { REQUIRE(e == InvalidIPv6AddressError()); }, {},
        Reactor::global());
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

    loop_with_initial_event_and_connectivity([]() {
        query("IN", "A", "www.neubot.org", [](Error e, Message message) {
            REQUIRE(!e);
            REQUIRE(message.error_code == DNS_ERR_NONE);
            REQUIRE(message.answers.size() == 1);
            REQUIRE(message.answers[0].ipv4 == "130.192.16.172");
            REQUIRE(message.rtt > 0.0);
            REQUIRE(message.answers[0].ttl > 0);
            break_loop();
        });
    });

    loop_with_initial_event_and_connectivity([]() {
        query(
            "IN", "REVERSE_A", "130.192.16.172", [](Error e, Message message) {
                REQUIRE(!e);
                REQUIRE(message.error_code == DNS_ERR_NONE);
                REQUIRE(message.answers.size() == 1);
                REQUIRE(message.answers[0].hostname == "server-nexa.polito.it");
                REQUIRE(message.rtt > 0.0);
                REQUIRE(message.answers[0].ttl > 0);
                break_loop();
            });
    });

    loop_with_initial_event_and_connectivity([]() {
        query("IN", "PTR", "172.16.192.130.in-addr.arpa.", [](Error e,
                                                              Message message) {
            REQUIRE(!e);
            REQUIRE(message.error_code == DNS_ERR_NONE);
            REQUIRE(message.answers.size() == 1);
            REQUIRE(message.answers[0].hostname == "server-nexa.polito.it");
            REQUIRE(message.rtt > 0.0);
            REQUIRE(message.answers[0].ttl > 0);
            break_loop();
        });
    });

    loop_with_initial_event_and_connectivity([]() {
        query("IN", "AAAA", "ooni.torproject.org",
              [](Error e, Message message) {
                  REQUIRE(!e);
                  REQUIRE(message.error_code == DNS_ERR_NONE);
                  REQUIRE(message.answers.size() > 0);
                  REQUIRE(message.rtt > 0.0);
                  REQUIRE(message.answers[0].ttl > 0);
                  auto found = false;
                  for (auto answer : message.answers) {
                      if (answer.ipv6 == "2001:858:2:2:aabb::563b:1e28" or
                          answer.ipv6 == "2001:858:2:2:aabb:0:563b:1e28") {
                          found = true;
                      }
                  }
                  REQUIRE(found);
                  break_loop();
              });
    });

    loop_with_initial_event_and_connectivity([]() {
        query("IN", "REVERSE_AAAA", "2001:858:2:2:aabb::563b:1e28",
              [](Error e, Message message) {
                  REQUIRE(!e);
                  REQUIRE(message.error_code == DNS_ERR_NONE);
                  REQUIRE(message.answers.size() == 1);
                  REQUIRE(message.answers[0].hostname == "nova.torproject.org");
                  REQUIRE(message.rtt > 0.0);
                  REQUIRE(message.answers[0].ttl > 0);
                  break_loop();
              });
    });

    loop_with_initial_event_and_connectivity([]() {
        query("IN", "PTR", "8.2.e.1.b.3.6.5.0.0.0.0.b.b.a.a.2.0.0.0.2.0.0.0.8."
                           "5.8.0.1.0.0.2.ip6.arpa",
              [](Error e, Message message) {
                  REQUIRE(!e);
                  REQUIRE(message.error_code == DNS_ERR_NONE);
                  REQUIRE(message.answers.size() == 1);
                  REQUIRE(message.answers[0].hostname == "nova.torproject.org");
                  REQUIRE(message.rtt > 0.0);
                  REQUIRE(message.answers[0].ttl > 0);
                  break_loop();
              });
    });
}
