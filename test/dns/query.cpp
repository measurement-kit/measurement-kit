// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/dns/resolve_hostname.hpp"
#include "src/libmeasurement_kit/dns/libevent_query.hpp"

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

static int null_evdns_base_nameserver_sockaddr_add(evdns_base *, const sockaddr *,
                                                   ev_socklen_t, unsigned) {
    return -1;
}

static int null_evdns_base_set_option_randomize(evdns_base *, const char *,
                                                const char *) {
    return -1;
}

#define BASE_FREE(name)                                                        \
    static bool base_free_##name##_flag = false;                               \
    struct evdns_base_##name##_deleter : private evdns_base_uptr::deleter_type { \
        void operator()(evdns_base_uptr::pointer p) {                            \
            evdns_base_uptr::deleter_type::operator()(p);                        \
            base_free_##name##_flag = true;                                      \
        }                                                                        \
    };                                                                           \
    using base_free_##name = std::unique_ptr<evdns_base_uptr::element_type, evdns_base_##name##_deleter>;

BASE_FREE(evdns_base_nameserver_sockaddr_add)
BASE_FREE(evdns_set_options_attempts)
BASE_FREE(evdns_set_options_attempts_negative)
BASE_FREE(evdns_set_options_timeout)
BASE_FREE(evdns_set_options_randomize)

TEST_CASE("throw error while fails evdns_base_new") {
    REQUIRE_THROWS_AS(
        create_evdns_base<null_evdns_base_new>({}, Reactor::global()),
        std::bad_alloc);
}

TEST_CASE("throw error on literal port") {
    // NB: dns/port=128000 just overflows uint16
    REQUIRE_THROWS_AS(
        (create_evdns_base(
            {{"dns/nameserver", "8.8.8.8"}, {"dns/port", "domain"}}, Reactor::global())),
        std::runtime_error);
}

TEST_CASE("throw error while fails evdns_base_nameserver_sockaddr_add") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, null_evdns_base_nameserver_sockaddr_add,
                           base_free_evdns_base_nameserver_sockaddr_add>(
            {{"dns/nameserver", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_base_nameserver_sockaddr_add_flag);
}

TEST_CASE("throw error while fails evdns_base_nameserver_sockaddr_add and base_new") {
    REQUIRE_THROWS_AS((create_evdns_base<null_evdns_base_new,
                                         null_evdns_base_nameserver_sockaddr_add>(
                          {{"dns/nameserver", "nexa"}}, Reactor::global())),
                      std::bad_alloc);
}

TEST_CASE("throw error while fails evdns_set_options for attempts") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_sockaddr_add,
                           base_free_evdns_set_options_attempts>(
            {{"dns/attempts", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_attempts_flag);
}

TEST_CASE("throw error while fails evdns_set_options for timeout") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_sockaddr_add,
                           base_free_evdns_set_options_timeout>(
            {{"dns/attempts", "nexa"}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_timeout_flag);
}

TEST_CASE("throw error while fails evdns_set_options for negative attempts") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_sockaddr_add,
                           base_free_evdns_set_options_attempts_negative>(
            {{"dns/attempts", -1}}, Reactor::global())),
        std::runtime_error);
    REQUIRE(base_free_evdns_set_options_attempts_negative_flag);
}

TEST_CASE("throw error while fails evdns_set_options for randomize-case") {
    REQUIRE_THROWS_AS(
        (create_evdns_base<::evdns_base_new, ::evdns_base_nameserver_sockaddr_add,
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
    libevent_query<::evdns_base_free, null_resolver>(
        "IN", "A", "www.google.com",
        [](Error e, SharedPtr<Message>) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_ipv6") {
    libevent_query<::evdns_base_free, ::evdns_base_resolve_ipv4, null_resolver>(
        "IN", "AAAA", "github.com",
        [](Error e, SharedPtr<Message>) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse") {
    libevent_query<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, null_resolver_reverse>(
        "IN", "REVERSE_A", "8.8.8.8",
        [](Error e, SharedPtr<Message>) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("dns::query deals with failing evdns_base_resolve_reverse_ipv6") {
    libevent_query<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                null_resolver_reverse>(
        "IN", "REVERSE_AAAA", "::1",

        [](Error e, SharedPtr<Message>) { REQUIRE(e == ResolverError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("dns::query deals with inet_pton returning 0") {
    libevent_query<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                ::evdns_base_resolve_reverse_ipv6, null_inet_pton>(
        "IN", "REVERSE_A", "8.8.8.8",

        [](Error e, SharedPtr<Message>) { REQUIRE(e == InvalidIPv4AddressError()); }, {},
        Reactor::global(), Logger::global());

    libevent_query<::evdns_base_free, ::evdns_base_resolve_ipv4,
                ::evdns_base_resolve_ipv6, ::evdns_base_resolve_reverse,
                ::evdns_base_resolve_reverse_ipv6, null_inet_pton>(
        "IN", "REVERSE_AAAA", "::1",
        [](Error e, SharedPtr<Message>) { REQUIRE(e == InvalidIPv6AddressError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("dns::query raises if the query is unsupported") {
    query("IN", "MX", "www.neubot.org",
          [](Error e, SharedPtr<Message>) { REQUIRE(e == UnsupportedTypeError()); },
          {{"dns/engine", "libevent"}});
}

TEST_CASE("dns::query raises if the class is unsupported") {
    query("CS", "A", "www.neubot.org",
          [](Error e, SharedPtr<Message>) { REQUIRE(e == UnsupportedClassError()); },
          {{"dns/engine", "libevent"}});
}

TEST_CASE("dns::query deals with invalid PTR name") {
    // This should be enough to see the failure, more tests for the
    // parser for PTR addresses are in test/common/utils.cpp
    query("IN", "PTR", "xx",
          [](Error e, SharedPtr<Message>) { REQUIRE(e == InvalidNameForPTRError()); },
          {{"dns/engine", "libevent"}});
}

#ifdef ENABLE_INTEGRATION_TESTS

// Test resolve_hostname

TEST_CASE("resolve_hostname works with IPv4 address") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        std::string hostname = "130.192.16.172";
        resolve_hostname(hostname, [=](ResolveHostnameResult r) {
            REQUIRE(r.inet_pton_ipv4);
            REQUIRE(r.addresses.size() == 1);
            REQUIRE(r.addresses[0] == hostname);
            reactor->stop();
        }, {}, reactor);
    });
}

TEST_CASE("resolve_hostname works with IPv6 address") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        std::string hostname = "2a00:1450:400d:807::200e";
        resolve_hostname(hostname, [=](ResolveHostnameResult r) {
            REQUIRE(r.inet_pton_ipv6);
            REQUIRE(r.addresses.size() == 1);
            REQUIRE(r.addresses[0] == hostname);
            reactor->stop();
        }, {}, reactor);
    });
}

TEST_CASE("resolve_hostname works with domain") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        resolve_hostname("google.com", [=](ResolveHostnameResult r) {
            REQUIRE(not r.inet_pton_ipv4);
            REQUIRE(not r.inet_pton_ipv6);
            REQUIRE(not r.ipv4_err);
            REQUIRE(not r.ipv6_err);
            // At least one IPv4 and one IPv6 addresses
            REQUIRE(r.addresses.size() > 1);
            reactor->stop();
        }, {}, reactor);
    });
}

TEST_CASE("stress resolve_hostname with invalid address and domain") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        // Pass input that is neither invalid IPvX nor valid domain
        resolve_hostname("192.1688.antani", [=](ResolveHostnameResult r) {
            REQUIRE(not r.inet_pton_ipv4);
            REQUIRE(not r.inet_pton_ipv6);
            REQUIRE(r.ipv4_err);
            REQUIRE(r.ipv6_err);
            REQUIRE(r.addresses.size() == 0);
            reactor->stop();
        }, {}, reactor);
    });
}

// Integration (or regress?) tests for dns::query.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The libevent resolver works as expected") {

    //
    // Note: this test also makes sure that we get sensible
    // response fields from the system resolver.
    //

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        query("IN", "A", "www.neubot.org", [=](Error e, SharedPtr<Message> message) {
            REQUIRE(!e);
            REQUIRE(message->error_code == DNS_ERR_NONE);
            REQUIRE(message->answers.size() == 1);
            REQUIRE(message->answers[0].ipv4 == "130.192.16.172");
            REQUIRE(message->rtt > 0.0);
            REQUIRE(message->answers[0].ttl > 0);
            reactor->stop();
        }, {{"dns/engine", "libevent"}}, reactor);
    });

    reactor->run_with_initial_event([=]() {
        query(
            "IN", "REVERSE_A", "130.192.16.172", [=](Error e, SharedPtr<Message> message) {
                REQUIRE(!e);
                REQUIRE(message->error_code == DNS_ERR_NONE);
                REQUIRE(message->answers.size() == 1);
                REQUIRE(message->answers[0].hostname == "server-nexa.polito.it");
                REQUIRE(message->rtt > 0.0);
                REQUIRE(message->answers[0].ttl > 0);
                reactor->stop();
            }, {{"dns/engine", "libevent"}}, reactor);
    });

    reactor->run_with_initial_event([=]() {
        query("IN", "PTR", "172.16.192.130.in-addr.arpa.", [=](Error e,
                                                             SharedPtr<Message> message) {
            REQUIRE(!e);
            REQUIRE(message->error_code == DNS_ERR_NONE);
            REQUIRE(message->answers.size() == 1);
            REQUIRE(message->answers[0].hostname == "server-nexa.polito.it");
            REQUIRE(message->rtt > 0.0);
            REQUIRE(message->answers[0].ttl > 0);
            reactor->stop();
        }, {{"dns/engine", "libevent"}}, reactor);
    });

    reactor->run_with_initial_event([=]() {
        query("IN", "AAAA", "ooni.torproject.org",
              [=](Error e, SharedPtr<Message> message) {
                  REQUIRE(!e);
                  REQUIRE(message->error_code == DNS_ERR_NONE);
                  REQUIRE(message->answers.size() > 0);
                  REQUIRE(message->rtt > 0.0);
                  REQUIRE(message->answers[0].ttl > 0);
                  auto found = false;
                  for (auto answer : message->answers) {
                      if (answer.ipv6 != "") {
                          found = true;
                      }
                  }
                  REQUIRE(found);
                  reactor->stop();
              }, {{"dns/engine", "libevent"}}, reactor);
    });

    reactor->run_with_initial_event([=]() {
        query("IN", "REVERSE_AAAA", "2001:858:2:2:aabb::563b:1e28",
              [=](Error e, SharedPtr<Message> message) {
                  REQUIRE(!e);
                  REQUIRE(message->error_code == DNS_ERR_NONE);
                  REQUIRE(message->answers.size() == 1);
                  REQUIRE(message->answers[0].hostname == "nova.torproject.org");
                  REQUIRE(message->rtt > 0.0);
                  REQUIRE(message->answers[0].ttl > 0);
                  reactor->stop();
              }, {{"dns/engine", "libevent"}}, reactor);
    });

    reactor->run_with_initial_event([=]() {
        query("IN", "PTR", "8.2.e.1.b.3.6.5.0.0.0.0.b.b.a.a.2.0.0.0.2.0.0.0.8."
                           "5.8.0.1.0.0.2.ip6.arpa",
              [=](Error e, SharedPtr<Message> message) {
                  REQUIRE(!e);
                  REQUIRE(message->error_code == DNS_ERR_NONE);
                  REQUIRE(message->answers.size() == 1);
                  REQUIRE(message->answers[0].hostname == "nova.torproject.org");
                  REQUIRE(message->rtt > 0.0);
                  REQUIRE(message->answers[0].ttl > 0);
                  reactor->stop();
              }, {{"dns/engine", "libevent"}}, reactor);
    });
}

#endif
