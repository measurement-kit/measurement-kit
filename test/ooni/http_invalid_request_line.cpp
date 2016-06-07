// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/http_invalid_request_line_impl.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;
using namespace mk::ooni;

TEST_CASE("The HTTP Invalid Request Line test should run") {
    Settings options;
    options["backend"] = "http://213.138.109.232/";
    HTTPInvalidRequestLineImpl http_invalid_request_line(options);
    loop_with_initial_event_and_connectivity([&]() {
        http_invalid_request_line.begin(
            [&]() { http_invalid_request_line.end([]() { break_loop(); }); });
    });
}

TEST_CASE(
    "The HTTP Invalid Request Line can manage a failure while connecting") {
    Settings options;
    options["backend"] = "http://213.138.109.232/";
    options["dns/nameserver"] = "8.8.8.1";
    options["dns/timeout"] = 0.1;
    HTTPInvalidRequestLineImpl http_invalid_request_line(options);
    loop_with_initial_event_and_connectivity([&]() {
        http_invalid_request_line.begin([&]() {
            http_invalid_request_line.end([]() { break_loop(); });
        });
    });
}
