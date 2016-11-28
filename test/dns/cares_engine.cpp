// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/dns/cares_engine_impl.hpp"

using namespace mk;

static int sendrecv_fail_count_count = 0;
static void sendrecv_fail_count(std::string, std::string, std::vector<uint8_t>,
                                Callback<Error, std::vector<uint8_t>> callback,
                                Settings, Var<Reactor>, Var<Logger>) {
    sendrecv_fail_count_count += 1;
    /*
     * Generally this would be MockedError, but we need to simulate a timeout
     * thus we're passing in the TimeoutError instead.
     */
    callback(dns::TimeoutError(), {});
}

TEST_CASE("cares engine deals correctly with dns/attempts option") {
    Var<Reactor> reactor = Reactor::make();

    SECTION("When the option is not a number") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code == dns::InvalidAttemptsOptionError().code);
                    reactor->break_loop();
                },
                {{"dns/attempts", "antani"}}, reactor, Logger::global());
        });
    }

    SECTION("When the option is a non-positive number") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code == dns::InvalidAttemptsOptionError().code);
                    reactor->break_loop();
                },
                {{"dns/attempts", 0}}, reactor, Logger::global());
        });
    }

    SECTION("In the common case") {
        REQUIRE(sendrecv_fail_count_count == 0);
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<dns::serialize, sendrecv_fail_count,
                                         dns::parse_into>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code == dns::TimeoutError().code);
                    reactor->break_loop();
                },
                {{"dns/attempts", 3}}, reactor, Logger::global(), 1);
        });
        REQUIRE(sendrecv_fail_count_count == 3);
    }
}

static ErrorOr<std::vector<uint8_t>>
serialize_and_check_17(std::string name, dns::QueryClass qclass,
                       dns::QueryType type, unsigned short qid, int rd,
                       Var<Logger> logger) {
    REQUIRE(qid == 17);
    return dns::serialize(name, qclass, type, qid, rd, logger);
}

static void sendrecv_fail(std::string, std::string, std::vector<uint8_t>,
                          Callback<Error, std::vector<uint8_t>> callback,
                          Settings, Var<Reactor>, Var<Logger>) {
    callback(MockedError(), {});
}

static bool evutil_random_called = false;
static void evutil_random_mock(void *p, size_t n) {
    REQUIRE(!evutil_random_called);
    evutil_random_called = true;
    evutil_secure_rng_get_bytes(p, n);
}

TEST_CASE("cares engine deals correctly with dns/query_id option") {
    Var<Reactor> reactor = Reactor::make();

    SECTION("When the option is not a number") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code == dns::InvalidQueryIdOptionError().code);
                    reactor->break_loop();
                },
                {{"dns/query_id", "antani"}}, reactor, Logger::global());
        });
    }

    SECTION("When the option is nonzero") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<serialize_and_check_17, sendrecv_fail>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    /*
                     * We make the test fail to avoid sending the query so
                     * here we actually _expect_ to see `MockedError`.
                     */
                    REQUIRE(err.code == MockedError().code);
                    reactor->break_loop();
                },
                {{"dns/query_id", 17}}, reactor, Logger::global(), 1);
        });
    }

    SECTION("When the option is zero") {
        REQUIRE(!evutil_random_called);
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<dns::serialize, sendrecv_fail,
                                         dns::parse_into, evutil_random_mock>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    /*
                     * We make the test fail to avoid sending the query so
                     * here we actually _expect_ to see `MockedError`.
                     */
                    REQUIRE(err.code == MockedError().code);
                    reactor->break_loop();
                },
                {{"dns/query_id", 0}}, reactor, Logger::global(), 1);
        });
        REQUIRE(evutil_random_called);
    }
}

static ErrorOr<std::vector<uint8_t>>
serialize_and_check_true(std::string name, dns::QueryClass qclass,
                         dns::QueryType type, unsigned short qid, int rd,
                         Var<Logger> logger) {
    REQUIRE(rd);
    return dns::serialize(name, qclass, type, qid, rd, logger);
}

static ErrorOr<std::vector<uint8_t>>
serialize_and_check_false(std::string name, dns::QueryClass qclass,
                          dns::QueryType type, unsigned short qid, int rd,
                          Var<Logger> logger) {
    REQUIRE(!rd);
    return dns::serialize(name, qclass, type, qid, rd, logger);
}

TEST_CASE("cares engine deals correctly with dns/recursion_desired option") {
    Var<Reactor> reactor = Reactor::make();

    SECTION("When the option is not a boolean") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code ==
                            dns::InvalidRecursionDesiredOptionError().code);
                    reactor->break_loop();
                },
                {{"dns/recursion_desired", "antani"}}, reactor,
                Logger::global());
        });
    }

    SECTION("When the option is true") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<serialize_and_check_true,
                                         sendrecv_fail>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    /*
                     * We make the test fail to avoid sending the query so
                     * here we actually _expect_ to see `MockedError`.
                     */
                    REQUIRE(err.code == MockedError().code);
                    reactor->break_loop();
                },
                {{"dns/recursion_desired", true}}, reactor, Logger::global(),
                1);
        });
    }

    SECTION("When the option is false") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<serialize_and_check_false,
                                         sendrecv_fail>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    /*
                     * We make the test fail to avoid sending the query so
                     * here we actually _expect_ to see `MockedError`.
                     */
                    REQUIRE(err.code == MockedError().code);
                    reactor->break_loop();
                },
                {{"dns/recursion_desired", false}}, reactor, Logger::global(),
                1);
        });
    }
}

static ErrorOr<std::vector<uint8_t>>
serialize_and_check_case(std::string name, dns::QueryClass qclass,
                         dns::QueryType type, unsigned short qid, int rd,
                         Var<Logger> logger) {
    REQUIRE(name == "google.com");
    return dns::serialize(name, qclass, type, qid, rd, logger);
}

TEST_CASE("cares engine deals correctly with dns/randomize_case option") {
    Var<Reactor> reactor = Reactor::make();

    SECTION("When the option is not a boolean") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code ==
                            dns::InvalidRandomizeCaseOptionError().code);
                    reactor->break_loop();
                },
                {{"dns/randomize_case", "antani"}}, reactor,
                Logger::global());
        });
    }

    SECTION("When the option is true") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    REQUIRE(err.code == dns::NotImplementedError().code);
                    reactor->break_loop();
                },
                {{"dns/randomize_case", true}}, reactor, Logger::global(),
                1);
        });
    }

    SECTION("When the option is false") {
        reactor->loop_with_initial_event([=]() {
            dns::cares_engine_query_impl<serialize_and_check_case,
                                         sendrecv_fail>(
                dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
                [=](Error err, Var<dns::Message>) {
                    /*
                     * We make the test fail to avoid sending the query so
                     * here we actually _expect_ to see `MockedError`.
                     */
                    REQUIRE(err.code == MockedError().code);
                    reactor->break_loop();
                },
                {{"dns/randomize_case", false}}, reactor, Logger::global(),
                1);
        });
    }
}

static ErrorOr<std::vector<uint8_t>>
serialize_fail(std::string, dns::QueryClass, dns::QueryType, unsigned short,
               int, Var<Logger>) {
    return MockedError();
}

TEST_CASE("cares engine deals with serialize() error") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query_impl<serialize_fail>(
            dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
            [=](Error err, Var<dns::Message>) {
                /*
                 * We make the test fail to avoid sending the query so
                 * here we actually _expect_ to see `MockedError`.
                 */
                REQUIRE(err.code == MockedError().code);
                reactor->break_loop();
            },
            {}, reactor, Logger::global(), 1);
    });
}

static ErrorOr<std::vector<uint8_t>>
serialize_okay(std::string, dns::QueryClass, dns::QueryType, unsigned short,
               int, Var<Logger>) {
    return std::vector<uint8_t>{};
}

/*
 * Note: this tests is redundant with tests above. Adding it for robustness.
 */
TEST_CASE("cares engine deals with sendrecv() error") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query_impl<serialize_okay, sendrecv_fail>(
            dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
            [=](Error err, Var<dns::Message>) {
                /*
                 * We make the test fail to avoid sending the query so
                 * here we actually _expect_ to see `MockedError`.
                 */
                REQUIRE(err.code == MockedError().code);
                reactor->break_loop();
            },
            {}, reactor, Logger::global(), 1);
    });
}

/*
 * Note: we already check above that we deal with TimeoutError and retry.
 */

static void sendrecv_okay(std::string, std::string, std::vector<uint8_t>,
                          Callback<Error, std::vector<uint8_t>> callback,
                          Settings, Var<Reactor>, Var<Logger>) {
    callback(NoError(), {});
}

static Error parse_into_fail(Var<dns::Message>, std::vector<uint8_t>,
                             Var<Logger>) {
    return MockedError();
}

TEST_CASE("cares engine deals with parse_into() error") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query_impl<serialize_okay, sendrecv_okay,
                                     parse_into_fail>(
            dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
            [=](Error err, Var<dns::Message>) {
                /*
                 * We make the test fail to avoid sending the query so
                 * here we actually _expect_ to see `MockedError`.
                 */
                REQUIRE(err.code == MockedError().code);
                reactor->break_loop();
            },
            {}, reactor, Logger::global(), 1);
    });
}

static Error parse_into_edit_qid(Var<dns::Message> message,
                                 std::vector<uint8_t>, Var<Logger>) {
    message->qid = 128;
    return NoError();
}

TEST_CASE("cares engine detects the case where the query-id is unexpected") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query_impl<serialize_okay, sendrecv_okay,
                                     parse_into_edit_qid>(
            dns::MK_DNS_CLASS_IN, dns::MK_DNS_TYPE_A, "google.com",
            [=](Error err, Var<dns::Message>) {
                /*
                 * We make the test fail to avoid sending the query so
                 * here we actually _expect_ to see `MockedError`.
                 */
                REQUIRE(err.code == dns::UnexpectedQueryIdError().code);
                reactor->break_loop();
            },
            {{"dns/query_id", 11}}, reactor, Logger::global(), 1);
    });
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("the cares resolver returns an error with an invalid_site") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query(
            "IN", "A", "invalid_site.antani",
            [=](Error e, Var<dns::Message> m) {
                REQUIRE(e == dns::NotExistError());
                REQUIRE(!!m);
                reactor->break_loop();
            },
            {}, reactor, Logger::global());
    });
}

TEST_CASE("the cares resolver is able to resolve an ipv4 address") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query("IN", "A", "www.neubot.org",
                                [=](Error e, Var<dns::Message> message) {
                                    REQUIRE(!e);
                                    REQUIRE(message->answers.size() == 1);
                                    REQUIRE(message->answers[0].ipv4 ==
                                            "130.192.16.172");
                                    reactor->break_loop();
                                },
                                {}, reactor, Logger::global());
    });
}

TEST_CASE("the cares resolver is able to resolve an ipv6 address") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query(
            "IN", "AAAA", "ooni.torproject.org",
            [=](Error e, Var<dns::Message> message) {
                REQUIRE(!e);
                REQUIRE(message->answers.size() > 0);
                auto found = false;
                for (auto answer : message->answers) {
                    /*
                     * Note: mapped IP address added because that is the reply
                     * received from Politecnico di Torino DNS.
                     */
                    if (answer.ipv6 == "2001:858:2:2:aabb::563b:1e28" or
                        answer.ipv6 == "2001:858:2:2:aabb:0:563b:1e28" or
                        answer.ipv6 == "::ffff:154.35.132.70") {
                        found = true;
                    }
                }
                REQUIRE(found);
                reactor->break_loop();
            },
            {}, reactor, Logger::global());
    });
}

TEST_CASE("the cares resolver cane resolve the canonical name") {
    Var<Reactor> reactor = Reactor::make();
    reactor->loop_with_initial_event([=]() {
        dns::cares_engine_query("IN", "CNAME", "ipv4.google.com",
                                [=](Error e, Var<dns::Message> message) {
                                    REQUIRE(!e);
                                    REQUIRE(message->answers.size() > 0);
                                    REQUIRE(message->answers[0].hostname ==
                                            "ipv4.l.google.com");
                                    reactor->break_loop();
                                },
                                {}, reactor, Logger::global());
    });
}

#endif
