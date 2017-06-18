// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

int main() {
    return mk::fapply(
        [](int x, int y, int z) {
            return x + y + z;
        },
        0, 7, 4
    );
}
