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
    TCPConnectImpl tcp_connect("test/fixtures/hosts.txt",
                               {
                                   {"port", "80"},
                               });
    loop_with_initial_event_and_connectivity([&]() {
        tcp_connect.begin([&]() { tcp_connect.end([]() { break_loop(); }); });
    });
}

TEST_CASE("The TCP connect test should fail with an invalid dns resolver") {
    TCPConnectImpl tcp_connect("test/fixtures/hosts.txt",
                               {{"host", "nexacenter.org"},
                                {"port", "80"},
                                {"dns/nameserver", "8.8.8.1"},
                                {"dns/attempts", 1},
                                {"dns/timeout", 0.0001}});
    loop_with_initial_event([&]() {
        tcp_connect.begin([&]() { tcp_connect.end([]() { break_loop(); }); });
    });
}
