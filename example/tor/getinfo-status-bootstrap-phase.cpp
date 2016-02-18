// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common/poller.hpp>
#include "src/tor.hpp"

using namespace mk;

static void check(Poller *poller, tor::Control ctrl, unsigned attempt = 0) {
    poller->call_soon([attempt, poller, ctrl]() {
        tor::getinfo_status_bootstrap_phase(
            ctrl, [attempt, poller, ctrl](Error error, int phase) {
                std::cout << (int)error << " " << phase << "\n";
                if (error || attempt > 0 || phase == 100) {
                    poller->call_later(1.0, [poller]() {
                        poller->break_loop();
                    });
                    return;
                }
                check(poller, ctrl, attempt + 1);
            });
    });
}

int main() {
    auto poller = Poller::tor();
    poller->call_soon([poller]() {
        tor::authenticate([poller](Error err, tor::Control ctrl) {
            std::cout << (int)err << "\n";
            if (err) {
                poller->break_loop();
                return;
            }
            tor::setconf_disable_network(ctrl, false,
                                         [ctrl, poller](Error err) {
                                             if (err) {
                                                 poller->break_loop();
                                                 return;
                                             }
                                             check(poller, ctrl);
                                         });
        }, "127.0.0.1", "9051", poller);
    });
    poller->loop();
}
