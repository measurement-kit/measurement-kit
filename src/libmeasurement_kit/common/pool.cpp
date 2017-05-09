// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

namespace mk {

Pool::Pool() {}

void Pool::gc() { dead_.clear(); }

Pool::~Pool() {
    gc();
    active_.clear();
}

} // namespace mk
