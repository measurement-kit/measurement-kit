// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/ooni.hpp>
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

TEST_CASE("The HTTP Invalid Request Line test should run") {
  measurement_kit::set_verbose(1);
  Settings options;
  options["backend"] = "http://google.com/";
  HTTPInvalidRequestLine http_invalid_request_line(options);
  http_invalid_request_line.begin([&](){
    http_invalid_request_line.end([](){
      measurement_kit::break_loop();
    });
  });
  measurement_kit::loop();
}
