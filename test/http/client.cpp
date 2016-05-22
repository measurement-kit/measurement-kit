// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>

using namespace mk;
using namespace mk::http;

// TODO: for now I have just refactored former http::Client tests to use
// instead the http::request() API, we should now remove duplicates!

// TODO: these tests should go in test/http/request.cpp

TEST_CASE("http::request() works as expected") {
    auto count = 0;
    loop_with_initial_event_and_connectivity([&]() {

        request(
            {
                {"http/url", "http://www.google.com/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"Connection", "close"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error, Var<Response> response) {
                std::cout << "Google:\r\n";
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

        request(
            {
                {"http/url", "http://www.neubot.org/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error, Var<Response> response) {
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

        request(
            {
                {"http/url", "http://www.torproject.org/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error, Var<Response> response) {
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

    });
}

TEST_CASE("http::request() works as expected over Tor") {
    auto count = 0;
    loop_with_initial_event_and_connectivity([&]() {

        request(
            {
                {"http/url", "http://www.google.com/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"Connection", "close"},
                {"net/socks5_proxy", "127.0.0.1:9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error error, Var<Response> response) {
                std::cout << "Error: " << (int)error << std::endl;
                std::cout << "Google:\r\n";
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

        request(
            {
                {"http/url", "http://www.neubot.org/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"net/socks5_proxy", "127.0.0.1:9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error error, Var<Response> response) {
                std::cout << "Error: " << (int)error << std::endl;
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

        request(
            {
                {"http/url", "http://www.torproject.org/robots.txt"},
                {"http/method", "GET"},
                {"http/http_version", "HTTP/1.1"},
                {"net/socks5_proxy", "127.0.0.1:9050"},
            },
            {
                {"Accept", "*/*"},
            },
            "", [&](Error error, Var<Response> response) {
                std::cout << "Error: " << (int)error << std::endl;
                std::cout << response->body.substr(0, 128) << "\r\n";
                std::cout << "[snip]\r\n";
                if (++count >= 3) {
                    break_loop();
                }
            });

    });
}

TEST_CASE("Make sure that we can access OONI's bouncer using httpo://...") {
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
                std::cout << "Error: " << (int)error << std::endl;
                std::cout << response->body << "\r\n";
                std::cout << "[snip]\r\n";
                break_loop();
            });

    });
}

TEST_CASE("Make sure that settings are not modified") {

    // XXX Since settings are copied when passed to request() I am not
    // sure now what was the purpose of this test, or whether this test
    // has always been conceptually wrong since it cannot fail.
    //
    // Leaving it here for now, added a comment to trigger future
    // investigation when we cleanup http tests in the aftermath of
    // the current wave of MK cleanups.

    Settings settings{
        {"http/url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"http/method", "POST"},
        {"http/http_version", "HTTP/1.1"},
        {"net/tor_socks_port", 9999},
    };

    loop_with_initial_event_and_connectivity([&]() {
        request(settings,
                {
                    {"Accept", "*/*"},
                },
                "{\"test-helpers\": [\"dns\"]}",
                [](Error error, Var<Response> response) {
                    // XXX: assumes that Tor is not running on port 9999
                    REQUIRE(error != 0);
                    std::cout << "Error: " << (int)error << std::endl;
                    std::cout << response->body << "\r\n";
                    std::cout << "[snip]\r\n";
                    break_loop();
                });
    });

    // Make sure that no changes were made
    for (auto &iter : settings) {
        auto ok = iter.first == "http/url" || iter.first == "http/method" ||
                  iter.first == "http/http_version" ||
                  iter.first == "net/tor_socks_port";
        REQUIRE(ok);
    }
}
