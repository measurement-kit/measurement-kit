// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/tcp_connect_impl.hpp"
#include <measurement_kit/common.hpp>

using namespace mk;
using namespace mk::ooni;

TEST_CASE(
    "The TCP connect test should run with an input file of DNS hostnames") {
    TCPConnectImpl tcp_connect("test/fixtures/hosts.txt", {
                                                       {"port", "80"},
                                                      });
    tcp_connect.begin(
        [&]() { tcp_connect.end([]() { mk::break_loop(); }); });
    mk::loop();
}

TEST_CASE("The TCP connect test should throw an exception if an invalid file "
          "path is given") {
    REQUIRE_THROWS_AS(
        TCPConnectImpl("/tmp/this-file-does-not-exist.txt", Settings()),
        InputFileDoesNotExist);
}

TEST_CASE(
    "The TCP connect test should throw an exception if no file path is given") {
    REQUIRE_THROWS_AS(TCPConnectImpl("", Settings()), InputFileRequired);
}

TEST_CASE("The TCP connect test should fail with an invalid dns resolver") {
    loop_with_initial_event([=]() {
        TCPConnectImpl tcp_connect("test/fixtures/hosts.txt",
                                      {{"host", "nexacenter.org"},
                                       {"port", "80"},
                                       {"dns/nameserver", "8.8.8.1"},
                                       {"dns/attempts", 1},
                                       {"dns/timeout", 0.0001}});
        tcp_connect.begin(
            [&]() { tcp_connect.end([]() { mk::break_loop(); }); });
    });
}
