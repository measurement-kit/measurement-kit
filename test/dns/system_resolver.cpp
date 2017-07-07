// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/dns/system_resolver.hpp"

using namespace mk;
using namespace mk::dns;

#ifdef ENABLE_INTEGRATION_TESTS
static const char *fail_inet_ntop(int, const void *, char *, socklen_t) {
    return nullptr;
}
#endif

static int fail_getaddrinfo(const char *, const char *, const struct addrinfo *,
                            struct addrinfo **) {
    return EAI_FAIL;
}

template <MK_MOCK(getaddrinfo), MK_MOCK(inet_ntop)>
void run_system_resolver(std::string dns_class, std::string dns_type,
                         std::string name, Callback<Error, Var<Message>> cb) {
    Var<Reactor> reactor = Reactor::make();
    Var<Logger> logger = Logger::make();
    reactor->run_with_initial_event([&]() {
        system_resolver<getaddrinfo, inet_ntop>(dns_class, dns_type, name,
                                                reactor, logger,
                                                [&](Error e, Var<Message> m) {
                                                    reactor->stop();
                                                    cb(e, m);
                                                });
    });
}

TEST_CASE("the system resolver can handle a getaddrinfo error") {
    run_system_resolver<fail_getaddrinfo>(
        "IN", "A", "www.neubot.org", [](Error e, Var<Message>) {
            REQUIRE(e == NonRecoverableFailureError());
        });
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("the system resolver can handle a inet_ntop error") {
    run_system_resolver<getaddrinfo, fail_inet_ntop>(
        "IN", "A", "neubot.org",
        [](Error e, Var<Message>) { REQUIRE(e == InetNtopFailureError()); });
}

#endif

TEST_CASE("the system resolver can handle an unsupported class") {
    run_system_resolver("CS", "A", "neubot.org", [](Error e, Var<Message>) {
        REQUIRE(e == UnsupportedClassError());
    });
}

TEST_CASE("the system resolver can handle an unsupported query type") {
    run_system_resolver("IN", "NS", "neubot.org", [](Error e, Var<Message>) {
        REQUIRE(e == UnsupportedTypeError());
    });
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("the system resolver returns an error with an invalid_site") {
    run_system_resolver(
        "IN", "A", "invalid_site.local", [](Error e, Var<Message>) {
            REQUIRE(e == HostOrServiceNotProvidedOrNotKnownError());
        });
}

TEST_CASE("the system resolver is able to resolve an ipv4 address") {
    run_system_resolver(
        "IN", "A", "www.neubot.org", [](Error e, Var<Message> message) {
            REQUIRE(!e);
            REQUIRE(message->answers.size() == 1);
            REQUIRE(message->answers[0].ipv4 == "130.192.16.172");
        });
}

TEST_CASE("the system resolver is able to resolve an ipv6 address") {
    run_system_resolver(
        "IN", "AAAA", "ooni.torproject.org", [](Error e, Var<Message> message) {
            REQUIRE(!e);
            REQUIRE(message->answers.size() > 0);
            auto found = false;
            for (auto answer : message->answers) {
                if (answer.ipv6 != "") {
                    found = true;
                }
            }
            REQUIRE(found);
        });
}

TEST_CASE("the system resolver can handle errors with a CNAME query") {
    run_system_resolver(
        "IN", "CNAME", "invalid_site.local", [](Error e, Var<Message>) {
            REQUIRE(e == HostOrServiceNotProvidedOrNotKnownError());
        });
}

TEST_CASE(
    "the system resolver doesn't resolve the canonical name with A query") {
    run_system_resolver("IN", "A", "ipv4.google.com",
                        [](Error e, Var<Message> message) {
                            REQUIRE(!e);
                            REQUIRE(message->answers.size() > 0);
                            REQUIRE(message->answers[0].hostname == "");
                        });
}

TEST_CASE("the system resolver is able to resolve the canonical name with "
          "CNAME query") {
    run_system_resolver(
        "IN", "CNAME", "ipv4.google.com", [](Error e, Var<Message> message) {
            REQUIRE(!e);
            REQUIRE(message->answers.size() > 0);
            REQUIRE(message->answers[0].hostname == "ipv4.l.google.com");
        });
}

#endif
