// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/ooni/tcp_connect.hpp"

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

TEST_CASE(
    "The TCP connect test should run with an input file of DNS hostnames") {
    TCPConnect tcp_connect("test/fixtures/hosts.txt", {
                                                       {"port", "80"},
                                                      });
    tcp_connect.begin(
        [&]() { tcp_connect.end([]() { measurement_kit::break_loop(); }); });
    measurement_kit::loop();
}

TEST_CASE("The TCP connect test should throw an exception if an invalid file "
          "path is given") {
    REQUIRE_THROWS_AS(
        TCPConnect("/tmp/this-file-does-not-exist.txt", Settings()),
        InputFileDoesNotExist);
}

TEST_CASE(
    "The TCP connect test should throw an exception if no file path is given") {
    REQUIRE_THROWS_AS(TCPConnect("", Settings()), InputFileRequired);
}
