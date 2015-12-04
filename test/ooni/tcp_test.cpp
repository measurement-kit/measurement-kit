// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ooni/tcp_test.hpp"
#include <measurement_kit/common.hpp>

#include <iostream>

using namespace mk;
using namespace mk::ooni;

TEST_CASE("TCPTest works as expected in a common case") {

    auto count = 0;
    TCPTest tcp_test("", Settings());

    auto client1 = tcp_test.connect(
        {
         {"host", "www.neubot.org"}, {"port", "80"},
        },
        [&count]() {
            if (++count >= 2) {
                mk::break_loop();
            }
        });

    auto client2 = tcp_test.connect(
        {
         {"host", "ooni.nu"}, {"port", "80"},
        },
        [&count]() {
            if (++count >= 2) {
                mk::break_loop();
            }
        });

    mk::loop();
}
