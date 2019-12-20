// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include "src/libmeasurement_kit/ooni/templates_impl.hpp"

using namespace mk;
using namespace mk::ooni;

TEST_CASE("dns query template works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        SharedPtr<nlohmann::json> entry(new nlohmann::json);
        templates::dns_query(
            entry, "A", "IN", "dns.google", "8.8.8.8:53",
            [=](Error err, SharedPtr<dns::Message>) {
                REQUIRE(!err);
                templates::dns_query(
                    entry, "A", "IN", "dns.google", "8.8.8.1:53",
                    [=](Error err, SharedPtr<dns::Message>) {
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
                        REQUIRE((query["hostname"] == "dns.google"));
                        answers = query["answers"];
                        REQUIRE(answers.is_array());
                        REQUIRE((answers[0]["ttl"].is_number()));
                        auto found = false;
                        for (auto &answer : answers) {
                          if ((found = (answer["ipv4"] == "8.8.8.8")) == true) {
                            break;
                          }
                        }
                        REQUIRE(found);
                        REQUIRE((answers[0]["answer_type"] == "A"));
                        /* Second query and response (should be error) */
                        query = queries[1];
                        REQUIRE((query["resolver_hostname"] == "8.8.8.1"));
                        resolver_port = query["resolver_port"];
                        REQUIRE((resolver_port == 53));
                        REQUIRE((query["failure"] != nullptr));
                        REQUIRE((query["query_type"] == "A"));
                        REQUIRE((query["hostname"] == "dns.google"));
                        answers = query["answers"];
                        REQUIRE(answers.is_array());
                        reactor->stop();
                    },
                    // Rationale: originally I assumed having an invalid DNS
                    // would have been enough for this test to fail. But it
                    // is a fact that many ISPs reply nonetheless. Among them
                    // Vodafone, which is now my ISP. Hence I become much
                    // more annoyed by this test being broken. Fix by using
                    // an insanely low timeout so it should always fail.
                    {{"dns/timeout", 0.0001}, {"dns/attempts", 1},
                     {"dns/engine", "libevent"}}, reactor, Logger::make());
            }, {{"dns/engine", "libevent"}}, reactor, Logger::make());
    });
}

TEST_CASE("dns query template works as expected with system engine") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        SharedPtr<nlohmann::json> entry(new nlohmann::json);
        templates::dns_query(entry, "A", "IN", "nexa.polito.it", "",
                [=](Error err, SharedPtr<dns::Message>) {
                    REQUIRE(!err);
                    nlohmann::json answers;
                    nlohmann::json root;
                    nlohmann::json query;
                    root = nlohmann::json::parse(entry->dump());
                    REQUIRE(root.is_object());
                    nlohmann::json queries = root["queries"];
                    REQUIRE(queries.is_array());
                    REQUIRE(queries.size() == 1);
                    /* First query and response (should be ok) */
                    query = queries[0];
                    REQUIRE((query["resolver_hostname"].is_null()));
                    REQUIRE((query["resolver_port"].is_null()));
                    REQUIRE((query["failure"] == nullptr));
                    REQUIRE((query["query_type"] == "A"));
                    REQUIRE((query["hostname"] == "nexa.polito.it"));
                    answers = query["answers"];
                    REQUIRE(answers.is_array());
                    REQUIRE(answers.size() == 2);
                    REQUIRE((answers[0]["ttl"].is_null()));
                    REQUIRE((answers[0]["hostname"] == "nexa.polito.it"));
                    REQUIRE((answers[0]["answer_type"] == "CNAME"));
                    REQUIRE((answers[1]["ttl"].is_null()));
                    REQUIRE((answers[1]["ipv4"] == "130.192.16.171"));
                    REQUIRE((answers[1]["answer_type"] == "A"));
                    reactor->stop();
                },
                {{"dns/engine", "system"}}, reactor, Logger::make());
    });
}

TEST_CASE("tcp connect returns error if port is missing") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        templates::tcp_connect({}, [=](Error err, SharedPtr<net::Transport> txp) {
            REQUIRE(err);
            REQUIRE(!!txp);
            reactor->stop();
        }, reactor, Logger::make());
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
                               }, reactor, Logger::make());
    });
}

TEST_CASE("http requests template works as expected") {
    SharedPtr<nlohmann::json> entry(new nlohmann::json);
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        templates::http_request(
            entry, {{"http/url", "https://httpbin.org/"},
                    {"net/ca_bundle_path", "cacert.pem"}}, {}, "",
            [=](Error err, SharedPtr<http::Response>) {
                REQUIRE(!err);
                templates::http_request(
                    entry, {{"http/url", "https://ooni.torproject.org:84/"},
                            {"net/timeout", 1.0}},
                    {}, "", [=](Error err, SharedPtr<http::Response>) {
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
                        REQUIRE((req["request"]["method"] != ""));
                        REQUIRE((req["response"]["body"] == nullptr));
                        REQUIRE((req["response"]["headers"].is_object()));
                        reactor->stop();
                    }, reactor, Logger::make());
            }, reactor, Logger::make());
    });
}

static void mocked_request(Settings settings, http::Headers,
        std::string, Callback<Error, SharedPtr<http::Response>> cb,
        SharedPtr<Reactor>, SharedPtr<Logger>,
        SharedPtr<http::Response>, int) {
    std::string probe_ip = settings.get("real_probe_ip_", std::string{});
    REQUIRE(probe_ip != "");
    SharedPtr<http::Response> response{new http::Response};
    response->request.reset(new http::Request);
    response->response_line = "HTTP/1.1 200 Ok";
    response->http_major = 1;
    response->http_minor = 1;
    response->status_code = 200;
    response->reason = "Ok";
    http::headers_push_back(response->headers, "Content-Type", "text/html");
    {
      std::stringstream ss;
      ss << "aaa " << probe_ip << " aaa";
      http::headers_push_back(response->headers,"X-IP-Address", ss.str());
    }
    {
      std::stringstream ss;
      ss << "<HTML><BODY>" << probe_ip << "</BODY></HTML>";
      response->body = ss.str();
    }
    cb(NoError(), std::move(response));
}

TEST_CASE("Http template scrubs IP addresses") {
    const char *ip = "1.1.1.1";

    auto test = [ip](Settings settings, Callback<SharedPtr<nlohmann::json>> &&cb) {
        using namespace mk::ooni::templates;
        SharedPtr<nlohmann::json> entry{new nlohmann::json};
        settings["real_probe_ip_"] = ip;
        http::Headers headers;
        std::string body = "";
        SharedPtr<Reactor> reactor = Reactor::make();
        SharedPtr<Logger> logger = Logger::make();
        http_request_impl<mocked_request>(entry, settings, headers,
                body, [entry, cb](Error error, SharedPtr<http::Response>) {
                    REQUIRE(error == NoError());
                    cb(entry);
                }, reactor, logger);
    };

    SECTION("By default the probe IP is scrubbed") {
        Settings settings;
        test(settings, [ip](SharedPtr<nlohmann::json> entry) {
            REQUIRE(entry->dump().find(ip) == std::string::npos);
        });
    }

    SECTION("IP is redacted when its inclusion is NOT requested") {
        Settings settings;
        settings["save_real_probe_ip"] = false;
        test(settings, [ip](SharedPtr<nlohmann::json> entry) {
            REQUIRE(entry->dump().find(ip) == std::string::npos);
        });
    }

    SECTION("IP is NOT redacted when its inclusion is requested") {
        Settings settings;
        settings["save_real_probe_ip"] = true;
        test(settings, [ip](SharedPtr<nlohmann::json> entry) {
            REQUIRE(entry->dump().find(ip) != std::string::npos);
        });
    }
}
