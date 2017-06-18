// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

int main() {
    auto retval = 0;
    mk::fapply_with_callback(
        [](int x, int y, int z, mk::Callback<int> &&cb) {
            cb(x + y + z);
        },
        [&retval](int value) {
            retval = value;
        },
        0, 7, 4
    );
    return retval;
}
