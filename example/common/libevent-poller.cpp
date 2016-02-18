// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <measurement_kit/common/poller.hpp>

using namespace mk;

int main() {
    auto poller = Poller::global();
    poller->call_soon([poller]() {
        poller->call_later(5.0, [poller]() {
            poller->break_loop();
        });
    });
    poller->loop();
}
