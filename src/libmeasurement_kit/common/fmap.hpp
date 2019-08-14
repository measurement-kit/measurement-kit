// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://programmers.stackexchange.com/a/170474>
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_FMAP_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_FMAP_HPP

#include <algorithm>
#include <functional>
#include <vector>

namespace mk {

template <typename A, typename B>
std::vector<B> fmap(std::vector<A> i, std::function<B(A)> f) {
    std::vector<B> o;
    std::transform(i.begin(), i.end(), std::back_inserter(o), f);
    return o;
}

} // namespace mk
#endif
