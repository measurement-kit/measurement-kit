// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_RANGE_HPP
#define MEASUREMENT_KIT_COMMON_RANGE_HPP

#include <numeric>
#include <vector>

namespace mk {

// See http://stackoverflow.com/a/17694752
template <typename T> std::vector<T> range(size_t count) {
    std::vector<T> vector(count);
    std::iota(vector.begin(), vector.end(), 0);
    return vector;
}

} // namespace mk
#endif
