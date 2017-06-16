// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/a/20441189>
#ifndef MEASUREMENT_KIT_COMMON_FAPPLY_HPP
#define MEASUREMENT_KIT_COMMON_FAPPLY_HPP

#include <tuple>
#include <utility>

namespace mk {

template <typename F, typename... A, std::size_t... I>
constexpr auto fapply_impl_(F &&f, std::tuple<A...> &&t,
                            std::index_sequence<I...>) {
    return f(std::get<I>(t)...);
}

// start compile time recursion to ensure the compiler generates code such
// that `f` is called with all the arguments inside `t`
template <typename F, typename... A>
constexpr auto fapply(F &&f, std::tuple<A...> &&t) {
    return fapply_impl_(f, std::move(t), std::index_sequence_for<A...>{});
}

// convert arbitrary arguments into a tuple
template <typename F, typename... A> constexpr auto fapply(F &&f, A &&... a) {
    return fapply(f, std::make_tuple(std::forward<A>(a)...));
}

} // namespace mk
#endif
