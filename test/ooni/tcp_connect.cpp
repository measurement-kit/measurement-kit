#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/ooni/tcp_connect.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

using namespace ight::common::settings;
using namespace ight::ooni::tcp_connect;

TEST_CASE("The TCP connect test should run with an input file of DNS hostnames") {
    ight_set_verbose(1);
    TCPConnect tcp_connect("test/fixtures/hosts.txt", {
        {"port", "80"},
    });
    tcp_connect.begin([&]() {
        tcp_connect.end([]() {
            ight_break_loop();
        });
    });
    ight_loop();
}

TEST_CASE("The TCP connect test should throw an exception if an invalid file path is given") {
  REQUIRE_THROWS_AS(
      TCPConnect("/tmp/this-file-does-not-exist.txt", Settings()),
      InputFileDoesNotExist
  );
}

TEST_CASE("The TCP connect test should throw an exception if no file path is given") {
  ight_set_verbose(1);
  REQUIRE_THROWS_AS(
      TCPConnect("", Settings()),
      InputFileRequired
  );
}
