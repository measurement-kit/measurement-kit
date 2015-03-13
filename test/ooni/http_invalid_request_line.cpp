#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/ooni/http_invalid_request_line.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

using namespace ight::ooni::http_invalid_request_line;

TEST_CASE("The HTTP Invalid Request Line test should run") {
  ight_set_verbose(1);
  ight::common::Settings options;
  options["backend"] = "http://google.com/";
  HTTPInvalidRequestLine http_invalid_request_line(options);
  http_invalid_request_line.begin([&](){
    http_invalid_request_line.end([](){
      ight_break_loop();
    });
  });
  ight_loop();
}
