// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/fapply.hpp"

#include <measurement_kit/common.hpp>

static inline void fapply_example() {
    auto r = mk::fapply([](int x, int y, int z) { return x + y + z; }, 0, 7, 4);
    if (r != 11) {
        throw std::runtime_error("invalid result");
    }
}

static inline void fapply_with_callback_example() {
    auto r = 0;
    mk::fapply_with_callback(
          [](int x, int y, int z, mk::Callback<int> &&cb) { cb(x + y + z); },
          [&r](int v) { r = v; }, 0, 7, 4);
    if (r != 11) {
        throw std::runtime_error("invalid result");
    }
}

int main() {
    fapply_example();
    fapply_with_callback_example();
}
