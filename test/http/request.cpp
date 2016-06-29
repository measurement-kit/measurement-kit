// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/common/check_connectivity.hpp"
#include "src/ext/Catch/single_include/catch.hpp"
#include "src/http/request_impl.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <openssl/md5.h>

using namespace mk;
using namespace mk::net;
using namespace mk::http;

// Either tor was running and hence everything should be OK, or tor was
// not running and hence connect() to socks port must have failed.
static inline bool check_error_after_tor(Error e) {
    return e == NoError() or e == ConnectFailedError();
}

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
            {"http/follow_redirects", "yes"},
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
            {"http/follow_redirects", "yes"},
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

TEST_CASE("http::request works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        request(
            {
                {"http/url",
                 "http://nexa.polito.it/nexafiles/ComeFunzionaInternet.pdf"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [](Error error, Var<Response> response) {
                REQUIRE(!error);
                REQUIRE(response->status_code == 200);
                REQUIRE(md5(response->body) ==
                        "efa2a8f1ba8a6335a8d696f91de69737");
                break_loop();
            });
    });
}

TEST_CASE("http::request() works using HTTPS") {
    loop_with_initial_event_and_connectivity([]() {
        request({{"http/url",
                  "https://didattica.polito.it/tesi/SaperComunicare.pdf"},
                 {"http/method", "GET"},
                 {"http/http_version", "HTTP/1.1"},
                 {"net/ca_bundle_path", "test/fixtures/certs.pem"}},
                {
                    {"Accept", "*/*"},
                },
                "",
                [](Error error, Var<Response> response) {
                    REQUIRE(!error);
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                            "1be9d96d157a3df328faa30e51faf63a");
                    break_loop();
                });
    });
}

TEST_CASE("http::request() works as expected over Tor") {
    loop_with_initial_event_and_connectivity([]() {
        request(
            {
                {"http/url",
                 "http://nexa.polito.it/nexafiles/ComeFunzionaInternet.pdf"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"Connection", "close"},
                {"net/socks5_proxy", "127.0.0.1:9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "",
            [&](Error error, Var<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                            "efa2a8f1ba8a6335a8d696f91de69737");
                }
                break_loop();
            });
    });
}

TEST_CASE("http::request correctly receives errors") {
    loop_with_initial_event_and_connectivity([]() {
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
            [](Error error, Var<Response> response) {
                REQUIRE(error);
                REQUIRE(response == nullptr);
                break_loop();
            });
    });
}

TEST_CASE("http::request_recv_response() behaves correctly when EOF "
          "indicates body END") {
    auto called = 0;

    loop_with_initial_event_and_connectivity([&]() {
        connect("nexa.polito.it", 80,
                [&](Error err, Var<Transport> transport) {
                    REQUIRE(!err);

                    request_recv_response(transport,
                                          [&called](Error, Var<Response>) {
                                              ++called;
                                              break_loop();
                                          });

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
                 {"net/dumb_transport", true}});
    });
    REQUIRE(called == 1);
}

#define SOCKS_PORT_IS(port)                                                    \
    static void socks_port_is_##port(                                          \
        std::string, int, Callback<Error, Var<Transport>>, Settings settings,  \
        Var<Logger>, Var<Reactor>) {                                           \
        REQUIRE(settings.at("net/socks5_proxy") == "127.0.0.1:" #port);        \
    }

static void socks_port_is_empty(std::string, int,
                                Callback<Error, Var<Transport>>,
                                Settings settings, Var<Logger>, Var<Reactor>) {
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
    request({}, {}, "",
            [&called](Error err, Var<Response>) {
                called = true;
                REQUIRE(err == MissingUrlError());
            });
    REQUIRE(called);
}

TEST_CASE("http::request_connect_impl() works for normal connections") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl({{"http/url", "http://www.google.com/robots.txt"}},
                             [](Error error, Var<Transport> transport) {
                                 REQUIRE(!error);
                                 REQUIRE(static_cast<bool>(transport));
                                 transport->close([]() { break_loop(); });
                             });
    });
}

TEST_CASE("http::request_send() works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [](Error error, Var<Transport> transport) {
                REQUIRE(!error);
                request_send(transport,
                             {
                                 {"http/method", "GET"},
                                 {"http/url", "http://www.google.com/"},
                             },
                             {}, "",
                             [transport](Error error) {
                                 REQUIRE(!error);
                                 transport->close([]() { break_loop(); });
                             });
            });
    });
}

static inline bool status_code_ok(int code) {
    return code == 302 or code == 200;
}

TEST_CASE("http::request_recv_response() works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [](Error error, Var<Transport> transport) {
                REQUIRE(!error);
                request_send(
                    transport,
                    {
                        {"http/method", "GET"},
                        {"http/url", "http://www.google.com/"},
                    },
                    {}, "",
                    [transport](Error error) {
                        REQUIRE(!error);
                        request_recv_response(
                            transport, [transport](Error e, Var<Response> r) {
                                REQUIRE(!e);
                                REQUIRE(status_code_ok(r->status_code));
                                REQUIRE(r->body.size() > 0);
                                transport->close([]() { break_loop(); });
                            });
                    });
            });
    });
}

TEST_CASE("http::request_sendrecv() works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [](Error error, Var<Transport> transport) {
                REQUIRE(!error);
                request_sendrecv(transport,
                                 {
                                     {"http/method", "GET"},
                                     {"http/url", "http://www.google.com/"},
                                 },
                                 {}, "",
                                 [transport](Error error, Var<Response> r) {
                                     REQUIRE(!error);
                                     REQUIRE(status_code_ok(r->status_code));
                                     REQUIRE(r->body.size() > 0);
                                     transport->close([]() { break_loop(); });
                                 });
            });
    });
}

TEST_CASE("http::request_sendrecv() works for multiple requests") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [](Error error, Var<Transport> transport) {
                REQUIRE(!error);
                request_sendrecv(
                    transport,
                    {
                        {"http/method", "GET"},
                        {"http/url", "http://www.google.com/"},
                    },
                    {}, "",
                    [transport](Error error, Var<Response> r) {
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
                            [transport](Error error, Var<Response> r) {
                                REQUIRE(!error);
                                REQUIRE(r->status_code == 200);
                                REQUIRE(r->body.size() > 0);
                                transport->close([]() { break_loop(); });
                            });
                    });
            });
    });
}

TEST_CASE("http::request() works as expected using httpo URLs") {
    loop_with_initial_event_and_connectivity([]() {
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
            [](Error error, Var<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    nlohmann::json body = nlohmann::json::parse(response->body);
                    REQUIRE(body["default"]["collector"] ==
                            "httpo://ihiderha53f36lsd.onion");
                    REQUIRE(body["dns"]["collector"] ==
                            "httpo://ihiderha53f36lsd.onion");
                    REQUIRE(body["dns"]["address"] == "213.138.109.232:57004");
                }
                break_loop();
            });
    });
}

TEST_CASE("http::request() works as expected using tor_socks_port") {
    loop_with_initial_event_and_connectivity([]() {
        request(
            {
                {"http/url",
                 "http://nexa.polito.it/nexafiles/ComeFunzionaInternet.pdf"},
                {"http/method", "POST"},
                {"http/http_version", "HTTP/1.1"},
                {"net/tor_socks_port", "9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "{\"test-helpers\": [\"dns\"]}",
            [](Error error, Var<Response> response) {
                REQUIRE(check_error_after_tor(error));
                if (!error) {
                    REQUIRE(response->status_code == 200);
                    REQUIRE(md5(response->body) ==
                            "efa2a8f1ba8a6335a8d696f91de69737");
                }
                break_loop();
            });
    });
}

TEST_CASE("http::request_connect_impl fails without an url") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl({},
                             [](Error error, Var<Transport>) {
                                 REQUIRE(error == MissingUrlError());
                                 break_loop();
                             });
    });
}

TEST_CASE("http::request_connect_impl fails with an uncorrect url") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl({{"http/url", ">*7\n\n"}},
                             [](Error error, Var<Transport>) {
                                 REQUIRE(error == UrlParserError());
                                 break_loop();
                             });
    });
}

TEST_CASE("http::request_send fails without url in settings") {
    loop_with_initial_event_and_connectivity([]() {
        request_connect_impl(
            {{"http/url", "http://www.google.com/"}},
            [](Error error, Var<Transport> transport) {
                REQUIRE(!error);
                request_send(transport, {{"http/method", "GET"}}, {}, "",
                             [transport](Error error) {
                                 REQUIRE(error == MissingUrlError());
                                 transport->close([]() { break_loop(); });
                             });
            });
    });
}

TEST_CASE("http::request() fails if fails request_send()") {
    loop_with_initial_event_and_connectivity([]() {
        request({{"http/method", "GET"}}, {}, "",
                [](Error error, Var<Response>) {
                    REQUIRE(error);
                    break_loop();
                });
    });
}
