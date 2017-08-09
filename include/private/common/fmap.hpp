// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://programmers.stackexchange.com/a/170474>
#ifndef PRIVATE_COMMON_FMAP_HPP
#define PRIVATE_COMMON_FMAP_HPP

#include <algorithm>
#include <functional>
#include <vector>

namespace mk {

template <typename A, typename B>
std::vector<B> fmap(std::vector<A> i, std::function<B(A)> f) {
    std::vector<B> o;
    std::transform(i.begin(), i.end(), std::back_inserter(o), f);
    return std::move(o);
}

} // namespace mk
#endif
