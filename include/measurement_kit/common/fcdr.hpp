// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/questions/10626856>
#ifndef MEASUREMENT_KIT_COMMON_FCDR_HPP
#define MEASUREMENT_KIT_COMMON_FCDR_HPP

#include <tuple>
#include <utility>

namespace mk {

template <typename... T, std::size_t... I>
constexpr auto fcdr_impl_(const std::tuple<T...> &t,
                          std::index_sequence<I...>) {
    // Note: using `+1` so we skip the first element of the original tuple
    return std::make_tuple(std::get<I + 1>(t)...);
}

template <typename T, typename... U>
constexpr auto fcdr(const std::tuple<T, U...> &t) {
    return fcdr_impl_(t, std::make_index_sequence<sizeof...(U)>{});
}

} // namespace mk
#endif
