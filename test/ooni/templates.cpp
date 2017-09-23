// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/ooni.hpp>

using namespace mk;
using namespace mk::ooni;
using namespace mk::report;

TEST_CASE("dns query template works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        SharedPtr<Entry> entry(new Entry);
        templates::dns_query(
            entry, "A", "IN", "nexa.polito.it", "8.8.8.8:53",
            [=](Error err, SharedPtr<dns::Message>) {
                REQUIRE(!err);
                templates::dns_query(
                    entry, "A", "IN", "nexa.polito.it", "8.8.8.1:53",
                    [=](Error err, SharedPtr<dns::Message>) {
                        REQUIRE(!!err);
                        Json answers;
                        Json root;
                        Json query;
                        int resolver_port;
                        root = Json::parse(entry->dump());
                        REQUIRE(root.is_object());
                        Json queries = root["queries"];
                        REQUIRE(queries.is_array());
                        REQUIRE(queries.size() == 2);
                        /* First query and response (should be ok) */
                        query = queries[0];
                        REQUIRE((query["resolver_hostname"] == "8.8.8.8"));
                        resolver_port = query["resolver_port"];
                        REQUIRE((resolver_port == 53));
                        REQUIRE((query["failure"] == nullptr));
                        REQUIRE((query["query_type"] == "A"));
                        REQUIRE((query["hostname"] == "nexa.polito.it"));
                        answers = query["answers"];
                        REQUIRE(answers.is_array());
                        REQUIRE((answers[0]["ttl"].is_number()));
                        REQUIRE((answers[0]["ipv4"] == "130.192.16.172"));
                        REQUIRE((answers[0]["answer_type"] >= "A"));
                        /* Second query and response (should be error) */
                        query = queries[1];
                        REQUIRE((query["resolver_hostname"] == "8.8.8.1"));
                        resolver_port = query["resolver_port"];
                        REQUIRE((resolver_port == 53));
                        REQUIRE((query["failure"] != nullptr));
                        REQUIRE((query["query_type"] == "A"));
                        REQUIRE((query["hostname"] == "nexa.polito.it"));
                        answers = query["answers"];
                        REQUIRE(answers.is_array());
                        reactor->stop();
                    },
                    {{"dns/timeout", 0.3}, {"dns/attempts", 1},
                     {"dns/engine", "libevent"}}, reactor);
            }, {{"dns/engine", "libevent"}}, reactor);
    });
}

TEST_CASE("tcp connect returns error if port is missing") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        templates::tcp_connect({}, [=](Error err, SharedPtr<net::Transport> txp) {
            REQUIRE(err);
            REQUIRE(!!txp);
            reactor->stop();
        }, reactor);
    });
}

TEST_CASE("tcp connect returns error if port is invalid") {
    SharedPtr<Reactor> reactor = Reactor::make();
    Settings settings;
    settings["port"] = "foobar";
    reactor->run_with_initial_event([=]() {
        templates::tcp_connect(settings,
                               [=](Error err, SharedPtr<net::Transport> txp) {
                                   REQUIRE(err);
                                   REQUIRE(!!txp);
                                   reactor->stop();
                               }, reactor);
    });
}

TEST_CASE("http requests template works as expected") {
    SharedPtr<Entry> entry(new Entry);
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        templates::http_request(
            entry, {{"http/url", "https://ooni.torproject.org/"}}, {}, "",
            [=](Error err, SharedPtr<http::Response>) {
                REQUIRE(!err);
                templates::http_request(
                    entry, {{"http/url", "https://ooni.torproject.org:84/"},
                            {"net/timeout", 1.0}},
                    {}, "", [=](Error err, SharedPtr<http::Response>) {
                        REQUIRE(err);
                        Json root;
                        Json requests;
                        Json req;
                        root = Json::parse(entry->dump());
                        REQUIRE((root["agent"] == "agent"));
                        REQUIRE((root["socksproxy"] == nullptr));
                        REQUIRE(root.is_object());
                        requests = root["requests"];
                        REQUIRE(requests.is_array());
                        REQUIRE(requests.size() == 2);
                        /* First request (should be ok) */
                        req = requests[0];
                        REQUIRE(req.is_object());
                        REQUIRE((req["failure"] == nullptr));
                        REQUIRE((req["response"]["body"].is_string()));
                        REQUIRE((req["response"]["body"].size() > 0));
                        REQUIRE((req["response"]["response_line"] ==
                                 "HTTP/1.1 200 OK"));
                        int code = req["response"]["code"];
                        REQUIRE((code == 200));
                        REQUIRE((req["response"]["headers"].is_object()));
                        REQUIRE((req["response"]["headers"].size() > 0));
                        /* Second request (should be error) */
                        req = requests[1];
                        REQUIRE(req.is_object());
                        REQUIRE((req["failure"] != nullptr));
                        reactor->stop();
                    }, reactor);
            }, reactor);
    });
}

#else
int main(){}
#endif
