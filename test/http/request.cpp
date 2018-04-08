// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/http/request_impl.hpp"
#include "src/libmeasurement_kit/net/error.hpp"

#include <openssl/md5.h>

using namespace mk;
using namespace mk::net;
using namespace mk::http;

#ifdef ENABLE_INTEGRATION_TESTS

// Either tor was running and hence everything should be OK, or tor was
// not running and hence connect() to socks port must have failed.
static inline bool check_error_after_tor(Error e) {
    return e == NoError() or e == ConnectionRefusedError();
}

/*
 * TODO: replace with common/utils.cpp sha256_of():
 */
static std::string md5(std::string s) {
    static const char *table[] = {"0", "1", "2", "3", "4", "5", "6", "7",
                                  "8", "9", "a", "b", "c", "d", "e", "f"};

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char *)s.data(), s.size(), digest);
    std::string retval;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        retval += table[((digest[i] & 0xf0) >> 4)];
        retval += table[(digest[i] & 0x0f)];
    }
    return retval;
}

#endif

/*
      _
  ___| | __ _ ___ ___
 / __| |/ _` / __/ __|
| (__| | (_| \__ \__ \
 \___|_|\__,_|___/___/

*/

TEST_CASE("HTTP Request class works as expected") {
    Request request;
    request.init(
        {
            {"http/max_redirects", 2},
            {"http/url",
             "http://www.example.com/antani?clacsonato=yes#melandri"},
            {"http/ignore_body", "yes"},
            {"http/method", "GET"},
            {"http/http_version", "HTTP/1.0"},
        },
        {
            {"User-Agent", "Antani/1.0.0.0"},
        },
        "0123456789");
    Buffer buffer;
    request.serialize(buffer);
    auto serialized = buffer.read();
    std::string expect = "GET /antani?clacsonato=yes HTTP/1.0\r\n";
    expect += "User-Agent: Antani/1.0.0.0\r\n";
    expect += "Host: www.example.com\r\n";
    expect += "Content-Length: 10\r\n";
    expect += "\r\n";
    expect += "0123456789";
    REQUIRE(serialized == expect);
}

TEST_CASE("HTTP Request class works as expected with explicit path") {
    Request request;
    request.init(
        {
            {"http/max_redirects", 2},
            {"http/url",
             "http://www.example.com/antani?clacsonato=yes#melandri"},
            {"http/path", "/antani?amicimiei"},
            {"http/ignore_body", "yes"},
            {"http/method", "GET"},
            {"http/http_version", "HTTP/1.0"},
        },
        {
            {"User-Agent", "Antani/1.0.0.0"},
        },
        "0123456789");
    Buffer buffer;
    request.serialize(buffer);
    auto serialized = buffer.read();
    std::string expect = "GET /antani?amicimiei HTTP/1.0\r\n";
    expect += "User-Agent: Antani/1.0.0.0\r\n";
    expect += "Host: www.example.com\r\n";
    expect += "Content-Length: 10\r\n";
    expect += "\r\n";
    expect += "0123456789";
    REQUIRE(serialized == expect);
}

/*
 _             _
| | ___   __ _(_) ___
| |/ _ \ / _` | |/ __|
| | (_) | (_| | | (__
|_|\___/ \__, |_|\___|
         |___/
*/

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("http::request works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://a.http.th.ooni.io/"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(md5(response->body) ==
                        "5d2182cb241b5a9aefad8ce584831666");
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("http::request() works using HTTPS") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request({{"http/url",
                  "https://didattica.polito.it/tesi/SaperComunicare.pdf"},
                 {"http/method", "GET"},
                 {"http/http_version", "HTTP/1.1"},
                 {"net/ca_bundle_path", "test/fixtures/saved_ca_bundle.pem"}},
                {
                    {"Accept", "*/*"},
                },
                "",
                [=](Error error, SharedPtr<Response> response) {
                    REQUIRE(!error);
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                            "1be9d96d157a3df328faa30e51faf63a");
                    reactor->stop();
                }, reactor);
    });
}

TEST_CASE("http::request() works as expected over Tor") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://a.http.th.ooni.io/"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"Connection", "close"},
                {"net/socks5_proxy", "127.0.0.1:9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                        "5d2182cb241b5a9aefad8ce584831666");
                }
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("http::request correctly receives errors") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://nexa.polito.it:81/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"net/timeout", "3.0"},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(error);
                REQUIRE(response == nullptr);
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("http::request_recv_response() behaves correctly when EOF "
          "indicates body END") {
    auto called = 0;

    SharedPtr<Logger> logger = Logger::make();
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([&]() {
        connect("nexa.polito.it", 80,
                [&](Error err, SharedPtr<Transport> transport) {
                    REQUIRE(!err);

                    request_recv_response(transport,
                                          [&](Error e, SharedPtr<Response> r) {
                                              REQUIRE(e == NoError());
                                              REQUIRE(r->status_code == 200);
                                              ++called;
                                              reactor->stop();
                                          }, {}, reactor, logger);

                    Buffer data;
                    data << "HTTP/1.1 200 Ok\r\n";
                    data << "Content-Type: text/plain\r\n";
                    data << "Connection: close\r\n";
                    data << "Server: Antani/1.0.0.0\r\n";
                    data << "\r\n";
                    data << "1234567";
                    transport->emit_data(data);
                    transport->emit_error(EofError());
                },
                {// With this connect() succeeds immediately and the
                 // callback receives a dumb Emitter transport that you
                 // can drive by calling its emit_FOO() methods
                 {"net/dumb_transport", true}}, reactor, logger);
    });
    REQUIRE(called == 1);
}

#endif // ENABLE_INTEGRATION_TESTS

TEST_CASE("http::request_recv_response() deals with immediate EOF") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        connect("xxx.antani", 0,
                [=](Error err, SharedPtr<Transport> transport) {
                    REQUIRE(!err);
                    request_recv_response(transport,
                                          [=](Error e, SharedPtr<Response> r) {
                                              REQUIRE(e == EofError());
                                              REQUIRE(!!r);
                                              reactor->stop();
                                          },
                                          {}, reactor);
                    transport->emit_error(EofError());
                },
                {// With this connect() succeeds immediately and the
                 // callback receives a dumb Emitter transport that you
                 // can drive by calling its emit_FOO() methods
                 {"net/dumb_transport", true}});
    });
}

#define SOCKS_PORT_IS(port)                                                    \
    static void socks_port_is_##port(                                          \
        std::string, int, Callback<Error, SharedPtr<Transport>>, Settings settings,  \
        SharedPtr<Reactor>, SharedPtr<Logger>) {                                           \
        REQUIRE(settings.at("net/socks5_proxy") == "127.0.0.1:" #port);        \
    }

static void socks_port_is_empty(std::string, int,
                                Callback<Error, SharedPtr<Transport>>,
                                Settings settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    REQUIRE(settings.find("net/socks5_proxy") == settings.end());
}

SOCKS_PORT_IS(9055)

TEST_CASE("Behavior is correct when only tor_socks_port is specified") {
    Settings settings{
        {"http/method", "POST"},
        {"http/http_version", "HTTP/1.1"},
        {"net/tor_socks_port", 9055},
    };

    settings["http/url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    request_connect_impl<socks_port_is_9055>(settings, nullptr);

    settings["http/url"] = "http://ooni.torproject.org/";
    request_connect_impl<socks_port_is_empty>(settings, nullptr);
}

SOCKS_PORT_IS(9999)

TEST_CASE("Behavior is correct with both tor_socks_port and socks5_proxy") {
    Settings settings{
        {"http/method", "POST"},
        {"http/http_version", "HTTP/1.1"},
        {"net/tor_socks_port", 9999},
        {"net/socks5_proxy", "127.0.0.1:9055"},
    };

    settings["http/url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    request_connect_impl<socks_port_is_9999>(settings, nullptr);

    settings["http/url"] = "http://ooni.torproject.org/";
    request_connect_impl<socks_port_is_9055>(settings, nullptr);
}

TEST_CASE("Behavior is corrent when only socks5_proxy is specified") {
    Settings settings{
        {"http/method", "POST"},
        {"http/http_version", "HTTP/1.1"},
        {"net/socks5_proxy", "127.0.0.1:9055"},
    };

    settings["http/url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    request_connect_impl<socks_port_is_9055>(settings, nullptr);

    settings["http/url"] = "http://ooni.torproject.org/";
    request_connect_impl<socks_port_is_9055>(settings, nullptr);
}

SOCKS_PORT_IS(9050)

TEST_CASE("Behavior is OK w/o tor_socks_port and socks5_proxy") {
    Settings settings{
        {"http/method", "POST"}, {"http_version", "HTTP/1.1"},
    };

    settings["http/url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    request_connect_impl<socks_port_is_9050>(settings, nullptr);

    settings["http/url"] = "http://ooni.torproject.org/";
    request_connect_impl<socks_port_is_empty>(settings, nullptr);
}

TEST_CASE("http::request() callback is called if input URL parsing fails") {
    bool called = false;
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=, &called]() {
        request({}, {}, "",
            [=, &called](Error err, SharedPtr<Response>) {
                called = true;
                REQUIRE(err == MissingUrlError());
                reactor->stop();
            }, reactor);
    });
    REQUIRE(called);
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("http::request_connect_impl() works for normal connections") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl({{"http/url", "http://www.google.com/robots.txt"}},
                             [=](Error error, SharedPtr<Transport> transport) {
                                 REQUIRE(!error);
                                 REQUIRE(static_cast<bool>(transport));
                                 transport->close([=]() { reactor->stop(); });
                             }, reactor);
    });
}

TEST_CASE("http::request_send() works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [=](Error error, SharedPtr<Transport> transport) {
                REQUIRE(!error);
                request_send(transport,
                             {
                                 {"http/method", "GET"},
                                 {"http/url", "http://www.google.com/"},
                             },
                             {}, "", Logger::global(),
                             [=](Error error, SharedPtr<Request> request) {
                                 REQUIRE((request->method == "GET"));
                                 REQUIRE((request->url.schema == "http"));
                                 REQUIRE((request->url.address ==
                                          "www.google.com"));
                                 REQUIRE((request->url.port == 80));
                                 REQUIRE((request->headers.size() == 0));
                                 REQUIRE((request->body == ""));
                                 REQUIRE(!error);
                                 transport->close([=]() { reactor->stop(); });
                             });
            }, reactor);
    });
}

static inline bool status_code_ok(int code) {
    return code == 302 or code == 200;
}

TEST_CASE("http::request_recv_response() works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [=](Error error, SharedPtr<Transport> transport) {
                REQUIRE(!error);
                request_send(
                    transport,
                    {
                        {"http/method", "GET"},
                        {"http/url", "http://www.google.com/"},
                    },
                    {}, "", Logger::global(),
                    [=](Error error, SharedPtr<Request>) {
                        REQUIRE(!error);
                        request_recv_response(
                            transport, [=](Error e, SharedPtr<Response> r) {
                                REQUIRE(!e);
                                REQUIRE(status_code_ok(r->status_code));
                                REQUIRE(r->body.size() > 0);
                                transport->close([=]() { reactor->stop(); });
                            }, {}, reactor);
                    });
            }, reactor);
    });
}

TEST_CASE("http::request_sendrecv() works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [=](Error error, SharedPtr<Transport> transport) {
                REQUIRE(!error);
                request_sendrecv(transport,
                                 {
                                     {"http/method", "GET"},
                                     {"http/url", "http://www.google.com/"},
                                 },
                                 {}, "",
                                 [=](Error error, SharedPtr<Response> r) {
                                     REQUIRE(!error);
                                     REQUIRE(status_code_ok(r->status_code));
                                     REQUIRE(r->body.size() > 0);
                                     transport->close([=]() { reactor->stop(); });
                                 }, reactor);
            }, reactor);
    });
}

TEST_CASE("http::request_sendrecv() works for multiple requests") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [=](Error error, SharedPtr<Transport> transport) {
                REQUIRE(!error);
                request_sendrecv(
                    transport,
                    {
                        {"http/method", "GET"},
                        {"http/url", "http://www.google.com/"},
                    },
                    {}, "",
                    [=](Error error, SharedPtr<Response> r) {
                        REQUIRE(!error);
                        REQUIRE(status_code_ok(r->status_code));
                        REQUIRE(r->body.size() > 0);
                        request_sendrecv(
                            transport,
                            {
                                {"http/method", "GET"},
                                {"http/url",
                                 "http://www.google.com/robots.txt"},
                            },
                            {}, "",
                            [=](Error error, SharedPtr<Response> r) {
                                REQUIRE(!error);
                                REQUIRE(r->status_code == 200);
                                REQUIRE(r->body.size() > 0);
                                transport->close([=]() { reactor->stop(); });
                            }, reactor);
                    }, reactor);
            }, reactor);
    });
}

TEST_CASE("http::request() works as expected using httpo URLs") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
                {"http/method", "POST"},
                {"http/http_version", "HTTP/1.1"},
            },
            {
                {"Accept", "*/*"},
            },
            "{\"test-helpers\": [\"dns\"]}",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    Json body = Json::parse(response->body);
                    auto check = [](std::string s) {
                        REQUIRE(s.substr(0, 8) == "httpo://");
                        REQUIRE(s.size() >= 6);
                        REQUIRE(s.substr(s.size() - 6) == ".onion");
                    };
                    check(body["default"]["collector"]);
                    check(body["dns"]["collector"]);
                    REQUIRE(body["dns"]["address"] == "37.218.247.110:57004");
                }
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("http::request() works as expected using tor_socks_port") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://a.http.th.ooni.io/"},
                {"http/method", "POST"},
                {"http/http_version", "HTTP/1.1"},
                {"net/tor_socks_port", "9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "{\"test-helpers\": [\"dns\"]}",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                            "c0eb138de5f70c3b37566ba88146465d");
                }
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("http::request() correctly follows redirects") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://fsrn.org"},
                {"http/max_redirects", 32},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(response->request->url.schema == "https");
                REQUIRE(response->request->url.address == "fsrn.org");
                REQUIRE(response->previous->status_code == 302);
                REQUIRE(response->previous->request->url.schema == "http");
                REQUIRE(response->previous->request->url.address == "fsrn.org");
                REQUIRE(!response->previous->previous);
                reactor->stop();
            }, reactor);
    });
}

TEST_CASE("Headers are preserved across redirects") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://httpbin.org/absolute-redirect/3"},
                {"http/max_redirects", 4},
            },
            {
                {"Spam", "Ham"}, {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(response->request->url.path == "/get");
                REQUIRE(response->previous->status_code == 302);
                REQUIRE(response->previous->request->url.path ==
                        "/absolute-redirect/1");
                auto body = Json::parse(response->body);
                REQUIRE(body["headers"]["Spam"] == "Ham");
                reactor->stop();
            },
            reactor);
    });
}

TEST_CASE("We correctly deal with end-of-response signalled by EOF") {
    /*
     * At the moment of writing this test, http://hushmail.com redirects to
     * https://hushmail.com closing the connection with EOF.
     *
     * See measurement-kit/ooniprobe-ios#79.
     */
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url", "http://hushmail.com"},
                {"http/max_redirects", 4},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(response->request->url.schema == "https");
                auto okay = response->request->url.address == "hushmail.com" ||
                        response->request->url.address == "www.hushmail.com";
                REQUIRE(okay);
                REQUIRE(response->previous->status_code / 100 == 3);
                REQUIRE(response->previous->request->url.schema == "http");
                reactor->stop();
            },
            reactor);
    });
}

TEST_CASE("We correctly deal with schema-less redirect") {
    /*
     * At the moment of writing this test, http://bacardi.com redirects to
     * //bacardi.com which used to confuse our redirect code.
     */
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request(
            {
                {"http/url",
                "https://httpbin.org/redirect-to?url=%2F%2Fhttpbin.org%2Fheaders"},
                {"http/max_redirects", 4},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [=](Error error, SharedPtr<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(response->request->url.schema == "https");
                REQUIRE(response->request->url.address == "httpbin.org");
                REQUIRE(response->request->url.path == "/headers");
                REQUIRE(response->previous->status_code / 100 == 3);
                REQUIRE(response->previous->request->url.path
                        == "/redirect-to");
                reactor->stop();
            },
            reactor);
    });
}

#endif // ENABLE_INTEGRATION_TESTS

TEST_CASE("http::request_connect_impl fails without an url") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl({},
                             [=](Error error, SharedPtr<Transport>) {
                                 REQUIRE(error == MissingUrlError());
                                 reactor->stop();
                             }, reactor);
    });
}

TEST_CASE("http::request_connect_impl fails with an uncorrect url") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl({{"http/url", ">*7\n\n"}},
                             [=](Error error, SharedPtr<Transport>) {
                                 REQUIRE(error == UrlParserError());
                                 reactor->stop();
                             }, reactor);
    });
}

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("http::request_send fails without url in settings") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [=](Error error, SharedPtr<Transport> transport) {
                REQUIRE(!error);
                request_send(transport, {{"http/method", "GET"}}, {}, "",
                             Logger::global(),
                             [=](Error error, SharedPtr<Request> request) {
                                 REQUIRE(!request);
                                 REQUIRE(error == MissingUrlError());
                                 transport->close([=]() { reactor->stop(); });
                             });
            }, reactor);
    });
}

#endif

TEST_CASE("http::request() fails if fails request_send()") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        request({{"http/method", "GET"}}, {}, "",
                [=](Error error, SharedPtr<Response>) {
                    REQUIRE(error);
                    reactor->stop();
                }, reactor);
    });
}

TEST_CASE("http::redirect() works as expected") {
    SECTION("When location starts with //") {
        REQUIRE(
            http::redirect(*http::parse_url_noexcept("http://www.x.org/f?x"),
                           "//www.y.com/bar")
                ->str() == "http://www.y.com/bar");
        REQUIRE(
            http::redirect(*http::parse_url_noexcept("https://www.x.org/f?x"),
                           "//www.y.com/bar")
                ->str() == "https://www.y.com/bar");
    }
    SECTION("When location starts with /") {
        REQUIRE(http::redirect(
                    *http::parse_url_noexcept("http://www.x.org/f?x"), "/bar")
                    ->str() == "http://www.x.org/bar");
        REQUIRE(http::redirect(
                    *http::parse_url_noexcept("https://www.x.org/f?x"), "/bar")
                    ->str() == "https://www.x.org/bar");
        REQUIRE(http::redirect(
                    *http::parse_url_noexcept("http://www.x.org:1/f?x"), "/bar")
                    ->str() == "http://www.x.org:1/bar");
        REQUIRE(
            http::redirect(*http::parse_url_noexcept("https://www.x.org:1/f?x"),
                           "/bar")
                ->str() == "https://www.x.org:1/bar");
        REQUIRE(http::redirect(*http::parse_url_noexcept("https://1.1.1.1/f?x"),
                               "/bar")
                    ->str() == "https://1.1.1.1/bar");
        REQUIRE(http::redirect(*http::parse_url_noexcept("http://[::1]/f?x"),
                               "/bar")
                    ->str() == "http://[::1]/bar");
        REQUIRE(http::redirect(*http::parse_url_noexcept("http://[::1]:66/f?x"),
                               "/bar")
                    ->str() == "http://[::1]:66/bar");
    }
    SECTION("When location is an absolute URL") {
        REQUIRE(http::redirect(*http::parse_url_noexcept("http://a.org/f?x"),
                               "https://b.org/b")
                    ->str() == "https://b.org/b");
        REQUIRE(http::redirect(*http::parse_url_noexcept("https://a.org/f?x"),
                               "http://b.org/b")
                    ->str() == "http://b.org/b");
    }
    SECTION("When location is a relative URL") {
        REQUIRE(http::redirect(*http::parse_url_noexcept("http://a.org/f"), "g")
                    ->str() == "http://a.org/f/g");
        REQUIRE(
            http::redirect(*http::parse_url_noexcept("http://a.org/f/"), "g")
                ->str() == "http://a.org/f/g");
        /*
         * Explicitly make sure that the old query is cleared.
         */
        REQUIRE(
            http::redirect(*http::parse_url_noexcept("https://a.org/f?x"), "g")
                ->str() == "https://a.org/f/g");
        REQUIRE(http::redirect(*http::parse_url_noexcept("https://a.org/f?x"),
                               "g?h")
                    ->str() == "https://a.org/f/g?h");
    }
}

static void fail_request(Settings, Headers, std::string,
                         Callback<Error, SharedPtr<Response>> cb,
                         SharedPtr<Reactor> = Reactor::global(),
                         SharedPtr<Logger> = Logger::global(),
                         SharedPtr<Response> = nullptr, int = 0) {
    cb(MockedError(), SharedPtr<Response>::make());
}

static void non_200_response(Settings, Headers, std::string,
                             Callback<Error, SharedPtr<Response>> cb,
                             SharedPtr<Reactor> = Reactor::global(),
                             SharedPtr<Logger> = Logger::global(),
                             SharedPtr<Response> = nullptr, int = 0) {
    SharedPtr<Response> response = SharedPtr<Response>::make();
    response->status_code = 500;
    response->body = "{}";
    cb(NoError(), response);
}

static void fail_parsing(Settings, Headers, std::string,
                         Callback<Error, SharedPtr<Response>> cb,
                         SharedPtr<Reactor> = Reactor::global(),
                         SharedPtr<Logger> = Logger::global(),
                         SharedPtr<Response> = nullptr, int = 0) {
    SharedPtr<Response> response = SharedPtr<Response>::make();
    response->status_code = 200;
    response->body = "{";
    cb(NoError(), response);
}

TEST_CASE("request_json_string() works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();

    SECTION("For underlying http::request() failure") {
        reactor->run_with_initial_event([=]() {
            request_json_string_impl<fail_request>(
              "GET", "http://www.google.com", "", {},
              [=](Error error, SharedPtr<Response>, Json) {
                  REQUIRE(error == MockedError());
                    reactor->stop();
              },
              {}, reactor, Logger::global());
        });
    }

    SECTION("For non-200 HTTP status code") {
        reactor->run_with_initial_event([=]() {
            request_json_string_impl<non_200_response>(
              "GET", "http://www.google.com", "", {},
              [=](Error error, SharedPtr<Response> resp, Json) {
                  REQUIRE(error == NoError());
                  REQUIRE(resp->status_code != 200);
                  reactor->stop();
              },
              {}, reactor, Logger::global());
        });
    }

    SECTION("For json_parse_and_process() error") {
        reactor->run_with_initial_event([=]() {
            request_json_string_impl<fail_parsing>(
              "GET", "http://www.google.com", "{}", {},
              [=](Error error, SharedPtr<Response>, Json) {
                  REQUIRE(error == JsonParseError());
              },
              {}, reactor, Logger::global());
        });
    }
}
