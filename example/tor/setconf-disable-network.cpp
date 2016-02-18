// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common/poller.hpp>
#include "src/tor.hpp"

using namespace mk;

int main() {
    auto poller = Poller::tor();
    poller->call_soon([poller]() {
        tor::authenticate([poller](Error err, tor::Control ctrl) {
            std::cout << (int)err << "\n";
            if (err) {
                poller->break_loop();
                return;
            }
            tor::setconf_disable_network(
                ctrl, false, [ctrl, poller](Error err) {
                    if (err) {
                        poller->break_loop();
                        return;
                    }
                    std::cout << (int)err << "\n";
                    poller->call_later(5.0, [ctrl, poller]() {
                        tor::setconf_disable_network(
                            ctrl, true, [ctrl, poller](Error err) {
                                std::cout << (int)err << "\n";
                                poller->break_loop();
                            });
                    });
                });
        }, "127.0.0.1", "9051", poller);
    });
    poller->loop();
}
