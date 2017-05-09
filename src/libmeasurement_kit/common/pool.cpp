// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common.hpp>

namespace mk {

Pool::Pool() {}

void Pool::query() {
    mk::warn("=== BEGIN POOL RESOURCES ===");
    for (auto &pair : resources_) {
        mk::warn("Resource '%s': use count: %ld", pair.second.c_str(),
                 pair.first.use_count());
    }
    mk::warn("=== END POOL RESOURCES ===");
}

Pool::~Pool() {
    query();
    resources_.clear();
}

} // namespace mk
