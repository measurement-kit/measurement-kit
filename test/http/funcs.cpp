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
    loop_with_initial_event_and_connectivity([]() {
        http::get("http://www.google.com/robots.txt",
                  [](Error error, Var<Response> response) {
                      std::cout << "Error: " << (int)error << "\r\n";
                      std::cout << response->body.substr(0, 128) << "\r\n";
                      std::cout << "[snip]\r\n";
                      break_loop();
                  });
    });
}

TEST_CASE("http::request() works as expected") {
    loop_with_initial_event_and_connectivity([]() {
        http::request({
                        {"http.method", "GET"},
                        {"http.url", "http://www.google.com/robots.txt"},
                      },
                      {},
                      "",
                      [](Error error, Var<Response> response) {
                          std::cout << "Error: " << (int)error << "\r\n";
                          std::cout << response->body.substr(0, 128) << "\r\n";
                          std::cout << "[snip]\r\n";
                          break_loop();
                      });
    });
}
