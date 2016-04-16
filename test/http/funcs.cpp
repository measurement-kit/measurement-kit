// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/http.hpp>

using namespace mk;
using namespace mk::http;

// TODO: these tests should go in test/http/request.cpp

TEST_CASE("http::get() works as expected") {
    http::get("http://www.google.com/robots.txt",
        [](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << "\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });
    mk::loop();
}

TEST_CASE("http::request() works as expected") {
    http::request("GET", "http://www.google.com/robots.txt",
        [](Error error, Response &&response) {
            std::cout << "Error: " << (int)error << "\r\n";
            std::cout << response.body.substr(0, 128) << "\r\n";
            std::cout << "[snip]\r\n";
            mk::break_loop();
        });
    mk::loop();
}
