// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Regression tests for `protocols/http.hpp` and `protocols/http.cpp`.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>

#include "src/http/request.hpp"

using namespace mk;
using namespace mk::net;
using namespace mk::http;

TEST_CASE("HTTP Request works as expected") {
    Request r(
        {
         {"url", "http://www.google.com/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            if (error != 0) {
                std::cout << "Error: " << (int)error << "\r\n";
                mk::break_loop();
                return;
            }
            std::cout << "HTTP/" << response.http_major << "."
                      << response.http_minor << " " << response.status_code
                      << " " << response.reason << "\r\n";
            for (auto &kv : response.headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });
    mk::loop();
}

TEST_CASE("HTTP request behaves correctly when EOF indicates body END") {

    auto called = 0;

    //
    // TODO: find a way to prevent a connection to nexa.polito.it when
    // this test run, possibly creating a stub for connect() just as
    // we created stubs for many libevent APIs.
    //

    Request r(
        {
         {"url", "http://nexa.polito.it/"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&called](Error, Response &&) { ++called; });

    auto stream = r.get_stream();
    auto transport = stream->get_transport();

    transport->emit_connect();

    Buffer data;
    data << "HTTP/1.1 200 Ok\r\n";
    data << "Content-Type: text/plain\r\n";
    data << "Connection: close\r\n";
    data << "Server: Antani/1.0.0.0\r\n";
    data << "\r\n";
    data << "1234567";
    transport->emit_data(data);
    transport->emit_error(NoError());

    REQUIRE(called == 1);
}

TEST_CASE("HTTP Request correctly receives errors") {
    Request r(
        {
         {"url", "http://nexa.polito.it:81/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"timeout", "3.0"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            if (error != 0) {
                std::cout << "Error: " << (int)error << "\r\n";
                mk::break_loop();
                return;
            }
            std::cout << "HTTP/" << response.http_major << "."
                      << response.http_minor << " " << response.status_code
                      << " " << response.reason << "\r\n";
            for (auto &kv : response.headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });
    mk::loop();
}

TEST_CASE("HTTP Request works as expected over Tor") {
    Request r(
        {
         {"url", "http://www.google.com/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"socks5_proxy", "127.0.0.1:9050"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            if (error != 0) {
                std::cout << "Error: " << (int)error << "\r\n";
                mk::break_loop();
                return;
            }
            std::cout << "HTTP/" << response.http_major << "."
                      << response.http_minor << " " << response.status_code
                      << " " << response.reason << "\r\n";
            for (auto &kv : response.headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });
    mk::loop();
}

TEST_CASE("Behavior is correct when only tor_socks_port is specified") {

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", 9055},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    Request r1{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    settings["url"] = "http://ooni.torproject.org/";
    Request r2{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9055");
    REQUIRE(r2.socks5_address() == "");
    REQUIRE(r2.socks5_port() == "");
}

TEST_CASE("Behavior is correct with both tor_socks_port and socks5_proxy") {

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", 9999},
        {"socks5_proxy", "127.0.0.1:9055"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    Request r1{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    settings["url"] = "http://ooni.torproject.org/";
    Request r2{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9999");
    REQUIRE(r2.socks5_address() == "127.0.0.1");
    REQUIRE(r2.socks5_port() == "9055");
}

TEST_CASE("Behavior is corrent when only socks5_proxy is specified") {

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9055"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    Request r1{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    settings["url"] = "http://ooni.torproject.org/";
    Request r2{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9055");
    REQUIRE(r2.socks5_address() == "127.0.0.1");
    REQUIRE(r2.socks5_port() == "9055");
}

TEST_CASE("Behavior is OK w/o tor_socks_port and socks5_proxy") {

    Settings settings{
        {"method", "POST"}, {"http_version", "HTTP/1.1"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    Request r1{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    settings["url"] = "http://ooni.torproject.org/";
    Request r2{settings,
               {
                {"Accept", "*/*"},
               },
               "{\"test-helpers\": [\"dns\"]}",
               [](Error, Response &&) {
                   /* nothing */
               }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9050");
    REQUIRE(r2.socks5_address() == "");
    REQUIRE(r2.socks5_port() == "");
}

TEST_CASE("The callback is called if input URL parsing fails") {
    bool called = false;
    Request r1({}, {}, "", [&called](Error err, Response) {
        called = true;
        REQUIRE(err == GenericError());
    });
    REQUIRE(called);
}

TEST_CASE("http::request_connect() works for normal connections") {
    loop_with_initial_event([]() {
        request_connect({
            {"url", "http://www.google.com/robots.txt"}
        }, [](Error error, Var<Transport> transport) {
            REQUIRE(!error);
            REQUIRE(static_cast<bool>(transport));
            break_loop();
        });
    });
}

TEST_CASE("http::request_send() works as expected") {
    loop_with_initial_event([]() {
        request_connect({
            {"url", "http://www.google.com/"}
        }, [](Error error, Var<Transport> transport) {
            REQUIRE(!error);
            request_send(transport, {
                {"method", "GET"},
                {"url", "http://www.google.com/"},
            }, {}, "", [](Error error) {
                REQUIRE(!error);
                break_loop();
            });
        });
    });
}

TEST_CASE("http::request_recv_response() works as expected") {
    loop_with_initial_event([]() {
        request_connect({
            {"url", "http://www.google.com/"}
        }, [](Error error, Var<Transport> transport) {
            REQUIRE(!error);
            request_send(transport, {
                {"method", "GET"},
                {"url", "http://www.google.com/"},
            }, {}, "", [transport](Error error) {
                REQUIRE(!error);
                request_recv_response(transport, [](Error e, Var<Response> r) {
                    REQUIRE(!e);
                    REQUIRE(r->status_code == 302);
                    REQUIRE(r->body.size() > 0);
                    break_loop();
                });
            });
        });
    });
}
