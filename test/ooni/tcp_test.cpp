// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/ooni.hpp>

using namespace mk;
using namespace mk::ooni;

TEST_CASE("TCPTestImpl works as expected in a common case") {
    auto count = 0;
    loop_with_initial_event_and_connectivity([&]() {

        templates::tcp_connect(
            {
                {"host", "www.neubot.org"}, {"port", "80"},
            },
            [&count](Error, Var<net::Transport> txp) {
                if (++count >= 3) {
                    txp->close([]() { break_loop(); });
                } else {
                    txp->close([]() {});
                }
            });

        templates::tcp_connect(
            {
                {"host", "ooni.nu"}, {"port", "80"},
            },
            [&count](Error, Var<net::Transport> txp) {
                if (++count >= 3) {
                    txp->close([]() { break_loop(); });
                } else {
                    txp->close([]() {});
                }
            });

        templates::tcp_connect(
                        {{"host", "ooni.nu"},
                          {"port", "80"},
                          {"dns/nameserver", "8.8.8.1"},
                          {"dns/timeout", 0.0001}},
                         [&count](Error err, Var<net::Transport>) {
                             REQUIRE(err);
                             if (++count >= 3) {
                                 break_loop();
                             }
                         });

    });
}
