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
        [&count](Error, Var<net::Transport> txp) {
            if (++count >= 3) {
                txp->close([]() { mk::break_loop(); });
            } else {
                txp->close([]() {});
            }
        });

    tcp_test.connect(
        {
         {"host", "ooni.nu"}, {"port", "80"},
        },
        [&count](Error, Var<net::Transport> txp) {
            if (++count >= 3) {
                txp->close([]() { mk::break_loop(); });
            } else {
                txp->close([]() {});
            }
        });

    tcp_test.connect(
        {
         {"host", "ooni.nu"}, {"port", "80"}, 
         {"dns/nameserver", "8.8.8.1"}, {"dns/timeout", 0.0001}
        },
        [&count](Error err, Var<net::Transport> txp) {
            REQUIRE(err);
            if (++count >= 3) {
                []() { mk::break_loop(); };
            }
        });
    mk::loop();
}
