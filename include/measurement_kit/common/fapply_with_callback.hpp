// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/a/20441189>
#ifndef MEASUREMENT_KIT_COMMON_FAPPLY_WITH_CALLBACK_HPP
#define MEASUREMENT_KIT_COMMON_FAPPLY_WITH_CALLBACK_HPP

#include <tuple>
#include <utility>

namespace mk {

template <typename F, typename CB, typename... A, std::size_t... I>
constexpr auto fapply_with_callback_(F &&f, CB &&cb, std::tuple<A...> &&t,
                                     std::index_sequence<I...>) {
    return f(std::move(std::get<I>(t))..., std::move(cb));
}

// start compile time recursion to ensure the compiler generates code such
// that `f` is called with all the arguments inside `t`
template <typename F, typename CB, typename... A>
constexpr auto fapply_with_callback(F &&f, CB &&cb, std::tuple<A...> &&t) {
    return fapply_with_callback_(std::move(f), std::move(cb), std::move(t),
                                 std::index_sequence_for<A...>{});
}

// convert arbitrary arguments into a tuple
template <typename F, typename CB, typename... A>
constexpr auto fapply_with_callback(F &&f, CB &&cb, A &&... a) {
    return fapply_with_callback(std::move(f), std::move(cb),
                                std::make_tuple(std::forward<A>(a)...));
}

} // namespace mk
#endif
