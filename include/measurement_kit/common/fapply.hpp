// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
// =============================================================
// Based on <http://stackoverflow.com/a/20441189>
#ifndef MEASUREMENT_KIT_COMMON_FAPPLY_HPP
#define MEASUREMENT_KIT_COMMON_FAPPLY_HPP

#include <measurement_kit/common/var.hpp>

#include <tuple>
#include <utility>

namespace mk {

/*
 * fapply()
 */

template <typename F, typename... A, std::size_t... I>
constexpr auto fapply_(F &&f, std::tuple<A...> &&t, std::index_sequence<I...>) {
    return f(std::move(std::get<I>(t))...);
}

// start compile time recursion to ensure the compiler generates code such
// that `f` is called with all the arguments inside `t`
template <typename F, typename... A>
constexpr auto fapply(F &&f, std::tuple<A...> &&t) {
    return fapply_(std::move(f), std::move(t), std::index_sequence_for<A...>{});
}

// convert arbitrary arguments into a tuple
template <typename F, typename... A> constexpr auto fapply(F &&f, A &&... a) {
    return fapply(std::move(f), std::make_tuple(std::forward<A>(a)...));
}

/*
 * fapply_with_callback()
 */

template <typename F, typename CB, typename... A, std::size_t... I>
constexpr void fapply_with_callback_(F &&f, CB &&cb, std::tuple<A...> &&t,
                                     std::index_sequence<I...>) {
    f(std::move(std::get<I>(t))..., std::move(cb));
}

// start compile time recursion to ensure the compiler generates code such
// that `f` is called with all the arguments inside `t`
template <typename F, typename CB, typename... A>
constexpr void fapply_with_callback(F &&f, CB &&cb, std::tuple<A...> &&t) {
    fapply_with_callback_(std::move(f), std::move(cb), std::move(t),
                          std::index_sequence_for<A...>{});
}

// convert arbitrary arguments into a tuple
template <typename F, typename CB, typename... A>
constexpr void fapply_with_callback(F &&f, CB &&cb, A &&... a) {
    fapply_with_callback(std::move(f), std::move(cb),
                         std::make_tuple(std::forward<A>(a)...));
}

/*
 * fapply_with_callback_and_route_exceptions()
 */

template <typename F, typename CB, typename EB, typename... A, std::size_t... I>
void fapply_with_callback_and_route_exceptions_(F &&f, CB &&cb, EB &&eb,
                                                std::tuple<A...> &&t,
                                                std::index_sequence<I...>) {
    auto route_exc = Var<bool>::make(true);
    try {
        f(std::move(std::get<I>(t))...,
          [ cb = std::move(cb), route_exc ](auto &&... f_out) {
              // This callback may be either called immediately or it may
              // be deferred. In the latter case, it would not be possible
              // anyway to route exceptions, because the current stack is
              // gone (hence the `Var<bool>`). In the former case, it is
              // not right to route exceptions, because we want to guarantee
              // that *either* `cb` or `eb` is called, not both.
              *route_exc = false;
              cb(std::move(f_out)...);
          });
    } catch (const std::exception &exc) {
        if (!*route_exc) {
            throw;
        }
        eb(exc);
    }
}

// start compile time recursion to ensure the compiler generates code such
// that `f` is called with all the arguments inside `t`
template <typename F, typename CB, typename EB, typename... A>
constexpr void fapply_with_callback_and_route_exceptions(F &&f, CB &&cb,
                                                         EB &&eb,
                                                         std::tuple<A...> &&t) {
    fapply_with_callback_and_route_exceptions_(std::move(f), std::move(cb),
                                               std::move(eb), std::move(t),
                                               std::index_sequence_for<A...>{});
}

// convert arbitrary arguments into a tuple
template <typename F, typename CB, typename EB, typename... A>
constexpr void fapply_with_callback_and_route_exceptions(F &&f, CB &&cb,
                                                         EB &&eb, A &&... a) {
    fapply_with_callback_and_route_exceptions(
          std::move(f), std::move(cb), std::move(eb),
          std::make_tuple(std::forward<A>(a)...));
}

} // namespace mk
#endif
