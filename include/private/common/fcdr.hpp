// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/questions/10626856>
#ifndef PRIVATE_COMMON_FCDR_HPP
#define PRIVATE_COMMON_FCDR_HPP

#include <tuple>
#include <utility>

namespace mk {

template <typename... T, std::size_t... I>
constexpr auto fcdr_impl_(std::tuple<T...> &&t, std::index_sequence<I...>) {
    // Note: using `+1` so we skip the first element of the original tuple
    return std::make_tuple(std::move(std::get<I + 1>(t))...);
}

template <typename T, typename... U>
constexpr auto fcdr(std::tuple<T, U...> &&t) {
    return fcdr_impl_(std::move(t), std::make_index_sequence<sizeof...(U)>{});
}

} // namespace mk
#endif
