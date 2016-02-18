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
        tor::authenticate([poller](Error error, tor::Control /*control*/) {
            std::cout << (int)error << "\n";
            poller->break_loop();
        }, "127.0.0.1", "9051", poller);
    });
    poller->loop();
}
