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

using namespace mk;
using namespace mk::http;

TEST_CASE("HTTP Client works as expected") {
    auto client = Client();
    auto count = 0;

    client.request(
        {
         {"url", "http://www.google.com/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"Connection", "close"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error, Response &&response) {
            std::cout << "Google:\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    client.request(
        {
         {"url", "http://www.neubot.org/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error, Response &&response) {
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    client.request(
        {
         {"url", "http://www.torproject.org/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error, Response &&response) {
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    mk::loop();
}

TEST_CASE("HTTP Client works as expected over Tor") {
    auto client = Client();
    auto count = 0;

    client.request(
        {
         {"url", "http://www.google.com/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"Connection", "close"},
         {"socks5_proxy", "127.0.0.1:9050"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << std::endl;
            std::cout << "Google:\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    client.request(
        {
         {"url", "http://www.neubot.org/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"socks5_proxy", "127.0.0.1:9050"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << std::endl;
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    client.request(
        {
         {"url", "http://www.torproject.org/robots.txt"},
         {"method", "GET"},
         {"http_version", "HTTP/1.1"},
         {"socks5_proxy", "127.0.0.1:9050"},
        },
        {
         {"Accept", "*/*"},
        },
        "", [&](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << std::endl;
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            if (++count >= 3) {
                mk::break_loop();
            }
        });

    mk::loop();
}

TEST_CASE("Make sure that we can access OONI's bouncer using httpo://...") {
    auto client = Client();

    client.request(
        {
         {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
         {"method", "POST"},
         {"http_version", "HTTP/1.1"},
        },
        {
         {"Accept", "*/*"},
        },
        "{\"test-helpers\": [\"dns\"]}", [](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << std::endl;
            std::cout << response.body << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });

    mk::loop();
}

TEST_CASE("Make sure that settings are not modified") {
    auto client = Client();

    Settings settings{
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", "9999"},
    };

    client.request(settings,
                   {
                    {"Accept", "*/*"},
                   },
                   "{\"test-helpers\": [\"dns\"]}",
                   [](Error error, Response &&response) {
                       // XXX: assumes that Tor is not running on port 9999
                       REQUIRE(error != 0);
                       std::cout << "Error: " << (int)error << std::endl;
                       std::cout << response.body << "\r\n";
                       std::cout << "[snip]\r\n";
                       mk::break_loop();
                   });

    mk::loop();

    // Make sure that no changes were made
    for (auto &iter : settings) {
        auto ok = iter.first == "url" || iter.first == "method" ||
                  iter.first == "http_version" ||
                  iter.first == "tor_socks_port";
        REQUIRE(ok);
    }
}
