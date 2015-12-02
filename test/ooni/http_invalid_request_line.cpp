// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/http_invalid_request_line.hpp"
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

TEST_CASE("The HTTP Invalid Request Line test should run") {
    Settings options;
    options["backend"] = "http://213.138.109.232/";
    HTTPInvalidRequestLine http_invalid_request_line(options);
    http_invalid_request_line.begin([&]() {
        http_invalid_request_line.end([]() { measurement_kit::break_loop(); });
    });
    measurement_kit::loop();
}
