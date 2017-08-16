// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/a/17694752>
#ifndef PRIVATE_COMMON_RANGE_HPP
#define PRIVATE_COMMON_RANGE_HPP

#include <numeric>
#include <vector>

namespace mk {

template <typename T> std::vector<T> range(size_t count) {
    std::vector<T> vector(count);
    std::iota(vector.begin(), vector.end(), 0);
    return vector;
}

} // namespace mk
#endif
