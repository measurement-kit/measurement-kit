// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/ooni/bouncer_impl.hpp"

using namespace mk;
using namespace mk::ooni;
using namespace mk::ooni::bouncer;

static void request_invalid(Settings, http::Headers, std::string,
                     Callback<Error, Var<http::Response>> cb,
                     Var<Reactor> = Reactor::global(),
                     Var<Logger> = Logger::global(),
                     Var<http::Response> = nullptr,
                     int = 0) {
    Var<http::Response> response(new http::Response());
    response->body = "{\"error\": \"invalid-request\"}";
    cb(NoError(), response);
}

TEST_CASE("The bouncer can handle an invalid request") {
    loop_with_initial_event([]() {
        // Mocked http request that returns an invalid-request
        post_net_tests_impl<request_invalid>(
            "https://a.collector.ooni.io/bouncer/net-tests", "web-connectivity",
            "0.0.1", {"web-connectivity"},
            [](Error e, Var<BouncerReply>) {
                REQUIRE(e == BouncerInvalidRequestError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("The bouncer can handle a collector not found response") {
    loop_with_initial_event([]() {
        post_net_tests_impl("https://a.collector.ooni.io/bouncer/net-tests",
                            "antani", "0.0.1", {"antani"},
                            [](Error e, Var<BouncerReply>) {

                                REQUIRE(e == BouncerCollectorNotFoundError());
                                break_loop();
                            },
                            {}, Reactor::global(), Logger::global());
    });
}

TEST_CASE("The bouncer works as expected when trying to get invalid values") {
    loop_with_initial_event([]() {
        post_net_tests_impl(
            "https://a.collector.ooni.io/bouncer/net-tests", "web-connectivity",
            "0.0.1", {"web-connectivity"},
            [](Error e, Var<BouncerReply> reply) {
                REQUIRE(!e);
                REQUIRE(reply->get_collector_alternate("antani").as_error() ==
                        BouncerValueNotFoundError());
                REQUIRE(reply->get_test_helper("antani").as_error() ==
                        BouncerValueNotFoundError());
                REQUIRE(reply->get_test_helper_alternate("web-connectivity", "antani").as_error() ==
                        BouncerValueNotFoundError());
                REQUIRE(reply->get_test_helper_alternate("antani", "cloudfront").as_error() ==
                        BouncerValueNotFoundError());
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}

TEST_CASE("The bouncer works as expected") {
    loop_with_initial_event([]() {
        post_net_tests_impl(
            "https://a.collector.ooni.io/bouncer/net-tests", "web-connectivity",
            "0.0.1", {"web-connectivity"},
            [](Error e, Var<BouncerReply> reply) {
                REQUIRE(!e);
                REQUIRE(*reply->get_collector() ==
                        "httpo://ihiderha53f36lsd.onion");
                REQUIRE(*reply->get_collector_alternate("https") ==
                        "https://a.collector.ooni.io:4441");
                REQUIRE(*reply->get_collector_alternate("cloudfront") ==
                        "https://das0y2z2ribx3.cloudfront.net");
                REQUIRE(*reply->get_name() == "web-connectivity");
                REQUIRE(*reply->get_test_helper("web-connectivity") ==
                        "httpo://7jne2rpg5lsaqs6b.onion");
                REQUIRE(*reply->get_test_helper_alternate("web-connectivity", "https")
                        == "https://a.web-connectivity.th.ooni.io:4442");
                REQUIRE(*reply->get_test_helper_alternate("web-connectivity", "cloudfront")
                        == "https://d2vt18apel48hw.cloudfront.net");
                break_loop();
            },
            {}, Reactor::global(), Logger::global());
    });
}
#endif
