// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/ooni/bouncer_impl.hpp"

using namespace mk;

static Error do_out_of_range(const std::string &, Callback<Json &> &&) {
    return JsonKeyError();
}

static Error do_domain_error(const std::string &, Callback<Json &> &&) {
    return JsonDomainError();
}

TEST_CASE("BouncerReply::create() works") {

    SECTION("When the collector is not found") {
        auto maybe_reply = ooni::BouncerReply::create(
            R"({"error": "collector-not-found"})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() ==
                ooni::BouncerCollectorNotFoundError());
    }

    SECTION("When the response is invalid") {
        auto maybe_reply = ooni::BouncerReply::create(
            R"({"error": "invalid-request"})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() == ooni::BouncerInvalidRequestError());
    }

    SECTION("When the error is something else") {
        auto maybe_reply = ooni::BouncerReply::create(
            R"({"error": "xx"})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() == ooni::BouncerGenericError());
    }

    SECTION("When the net-tests section is missing") {
        auto maybe_reply = ooni::BouncerReply::create(
            R"({})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() ==
                ooni::BouncerTestHelperNotFoundError());
    }

    SECTION("When the parser throws invalid_argument") {
        auto maybe_reply = ooni::BouncerReply::create(
            R"({)", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() == JsonParseError());
    }

    SECTION("When the parser throws out_of_range") {
        auto maybe_reply = ooni::bouncer::create_impl<do_out_of_range>(
            R"({})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() == JsonKeyError());
    }

    SECTION("When the parser throws domain_error") {
        auto maybe_reply = ooni::bouncer::create_impl<do_domain_error>(
            R"({})", Logger::global());
        REQUIRE(!maybe_reply);
        REQUIRE(maybe_reply.as_error() == JsonDomainError());
    }
}

TEST_CASE("BouncerReply accessors are robust to missing fields") {
    auto maybe_reply = ooni::BouncerReply::create(
        R"({"net-tests": [{"test-helpers-alternate":[], "collector-alternate":1234}]})",
        Logger::global());
    REQUIRE(!!maybe_reply);
    auto reply = *maybe_reply;

    SECTION("For get_collector") {
        auto maybe_value = reply->get_collector();
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }

    SECTION("For get_collector_alternate") {
        auto maybe_value = reply->get_collector_alternate("xx");
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }

    SECTION("For get_name") {
        auto maybe_value = reply->get_name();
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }

    SECTION("For get_test_helper") {
        auto maybe_value = reply->get_test_helper("xx");
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }

    SECTION("For get_test_helper_alternate") {
        auto maybe_value = reply->get_test_helper_alternate("xx", "yy");
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }

    SECTION("For get_version") {
        auto maybe_value = reply->get_version();
        REQUIRE(!maybe_value);
        REQUIRE(maybe_value.as_error() == ooni::BouncerValueNotFoundError());
    }
}

static void request_error(Settings, http::Headers, std::string,
                          Callback<Error, SharedPtr<http::Response>> cb,
                          SharedPtr<Reactor> = Reactor::global(),
                          SharedPtr<Logger> = Logger::global(),
                          SharedPtr<http::Response> = nullptr, int = 0) {
    cb(MockedError(), nullptr);
}

TEST_CASE("post_net_tests() works") {

    SECTION("On network error") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            // Mocked http request that returns an invalid-request
            ooni::bouncer::post_net_tests_impl<request_error>(
                ooni::bouncer::production_bouncer_url(), "web-connectivity",
                "0.0.1", {"web-connectivity"},
                [=](Error e, SharedPtr<ooni::BouncerReply>) {
                    REQUIRE(e == MockedError());
                    reactor->stop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When the collector is not found") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::bouncer::post_net_tests(
                ooni::bouncer::production_bouncer_url(), "antani", "0.0.1",
                {"antani"},
                [=](Error e, SharedPtr<ooni::BouncerReply>) {
                    REQUIRE(e == ooni::BouncerCollectorNotFoundError());
                    reactor->stop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When the input is correct") {
        SharedPtr<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=]() {
            ooni::bouncer::post_net_tests(
                ooni::bouncer::production_bouncer_url(), "web-connectivity",
                "0.0.1", {"web-connectivity"},
                [=](Error e, SharedPtr<ooni::BouncerReply> reply) {
                    REQUIRE(!e);
                    auto check_onion = [](std::string s) {
                        REQUIRE(s.substr(0, 8) == "httpo://");
                        REQUIRE(s.size() >= 6);
                        REQUIRE(s.substr(s.size() - 6) == ".onion");
                    };
                    auto check_https = [](std::string s) {
                        REQUIRE(s.substr(0, 8) == "https://");
                        REQUIRE(s.find("ooni.io") != std::string::npos);
                    };
                    auto check_cf = [](std::string s) {
                        REQUIRE(s.substr(0, 8) == "https://");
                        REQUIRE(s.find("cloudfront.net") != std::string::npos);
                    };

                    check_onion(*reply->get_collector());
                    check_https(*reply->get_collector_alternate("https"));
                    check_cf(*reply->get_collector_alternate("cloudfront"));
                    REQUIRE(*reply->get_name() == "web-connectivity");
                    check_onion(*reply->get_test_helper("web-connectivity"));
                    check_https(*reply->get_test_helper_alternate(
                                "web-connectivity", "https"));
                    check_cf(*reply->get_test_helper_alternate(
                                "web-connectivity", "cloudfront"));
                    reactor->stop();
                },
                {}, reactor, Logger::global());
        });
    }
}
