// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/tcp_test_impl.hpp"
#include <measurement_kit/common.hpp>

#include <iostream>

using namespace mk;
using namespace mk::ooni;

TEST_CASE("TCPTestImpl works as expected in a common case") {

    auto count = 0;
    TCPTestImpl tcp_test("", Settings());

    tcp_test.connect(
        {
         {"host", "www.neubot.org"}, {"port", "80"},
        },
        [&count](TCPClient) {
            if (++count >= 2) {
                mk::break_loop();
            }
        });

    tcp_test.connect(
        {
         {"host", "ooni.nu"}, {"port", "80"},
        },
        [&count](TCPClient) {
            if (++count >= 2) {
                mk::break_loop();
            }
        });

    mk::loop();
}
