// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/ooni.hpp>

using namespace mk;
using namespace mk::ooni;
using namespace mk::report;

TEST_CASE("dns query template works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        Var<Entry> entry(new Entry);
        templates::dns_query(
            entry, "A", "IN", "nexa.polito.it", "8.8.8.8:53",
            [=](Error err, dns::Message) {
                REQUIRE(!err);
                templates::dns_query(
                    entry, "A", "IN", "nexa.polito.it", "8.8.8.1:53",
                    [=](Error err, dns::Message) {
                        REQUIRE(!!err);
                        nlohmann::json answers;
                        nlohmann::json root;
                        nlohmann::json query;
                        int resolver_port;
                        root = nlohmann::json::parse(entry->dump());
                        REQUIRE(root.is_object());
                        nlohmann::json queries = root["queries"];
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
                        break_loop();
                    },
                    {{"dns/timeout", 0.3}, {"dns/attempts", 1}});
            });
    });
}

TEST_CASE("tcp connect returns error if port is missing") {
    loop_with_initial_event([]() {
        templates::tcp_connect({}, [](Error err, Var<net::Transport> txp) {
            REQUIRE(err);
            REQUIRE(txp == nullptr);
            break_loop();
        });
    });
}

TEST_CASE("tcp connect returns error if port is invalid") {
    Settings settings;
    settings["port"] = "foobar";
    loop_with_initial_event([=]() {
        templates::tcp_connect(settings,
                               [](Error err, Var<net::Transport> txp) {
                                   REQUIRE(err);
                                   REQUIRE(txp == nullptr);
                                   break_loop();
                               });
    });
}

TEST_CASE("http requests template works as expected") {
    Var<Entry> entry(new Entry);
    loop_with_initial_event_and_connectivity([=]() {
        templates::http_request(
            entry, {{"http/url", "http://nexa.polito.it/robots.txt"}}, {}, "",
            [=](Error err, Var<http::Response>) {
                REQUIRE(!err);
                templates::http_request(
                    entry, {{"http/url", "http://nexa.polito.it:84/robots.txt"},
                            {"net/timeout", 1.0}},
                    {}, "", [=](Error err, Var<http::Response>) {
                        REQUIRE(err);
                        nlohmann::json root;
                        nlohmann::json requests;
                        nlohmann::json req;
                        root = nlohmann::json::parse(entry->dump());
                        REQUIRE((root["agent"] == "agent"));
                        REQUIRE((root["socksproxy"] == nullptr));
                        REQUIRE(root.is_object());
                        requests = root["requests"];
                        REQUIRE(requests.is_array());
                        REQUIRE((requests.size() == 2));
                        /* First request (should be ok) */
                        req = requests[0];
                        REQUIRE(req.is_object());
                        REQUIRE((req["failure"] == nullptr));
                        REQUIRE((req["response"]["body"].is_string()));
                        REQUIRE((req["response"]["body"].size() > 0));
                        // FIXME: response line not saved
                        /*REQUIRE((req["response"]["response_line"] ==
                                 "HTTP/1.1 200 OK"));*/
                        int code = req["response"]["code"];
                        REQUIRE((code == 200));
                        REQUIRE((req["response"]["headers"].is_object()));
                        REQUIRE((req["response"]["headers"].size() > 0));
                        /* Second request (should be error) */
                        req = requests[1];
                        REQUIRE(req.is_object());
                        REQUIRE((req["failure"] != nullptr));
                        break_loop();
                    });
            });
    });
}
