// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/questions/10626856>
#ifndef PRIVATE_COMMON_FREVERSE_HPP
#define PRIVATE_COMMON_FREVERSE_HPP

#include <tuple>
#include <utility>

namespace mk {

template <typename... T, std::size_t... I>
constexpr auto freverse_(std::tuple<T...> &&t, std::index_sequence<I...>) {
    return std::make_tuple(std::move(std::get<sizeof...(T) - I - 1>(t))...);
}

template <typename... T> constexpr auto freverse(std::tuple<T...> &&t) {
    return freverse_(std::move(t), std::make_index_sequence<sizeof...(T)>{});
}

} // namespace mk
#endif
