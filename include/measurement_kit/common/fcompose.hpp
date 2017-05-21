// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP
#define MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP

#include <measurement_kit/common/fapply.hpp>
#include <measurement_kit/common/fcar.hpp>
#include <measurement_kit/common/fcdr.hpp>
#include <measurement_kit/common/freverse.hpp>

namespace mk {

// pathologic corner case
template <typename P, typename F, typename... G>
auto fcompose(P, F f, std::tuple<G...>, std::index_sequence<0>) {
    return f;
}

// base case
template <typename P, typename F, typename... G>
auto fcompose(P p, F f, std::tuple<G...> &&g, std::index_sequence<1>) {
    return p(f, fcar(g));
}

// generic case
template <typename P, typename F, typename... G, std::size_t I,
          typename = typename std::enable_if<(I >= 2)>::type>
auto fcompose(P p, F f, std::tuple<G...> &&g, std::index_sequence<I>) {
    return fcompose(p, p(f, fcar(g)), fcdr(g), std::index_sequence<I - 1>{});
}

/*
 * Set policy (sync, async), convert `g...` into a tuple, and start
 * compile time recursion with index sequence `len(g...)`.
 */

template <typename F, typename... G> auto fcompose_sync(F f, G &&... g) {
    return fcompose(
        [](auto f, auto g) {
            return [ f = std::move(f), g = std::move(g) ](auto... f_in) {
                /*
                 * Compose `f` and `g` by applying `args` to f and passing the
                 * return value (possibly a tuple to be expanded) as the
                 * arguments that must be applied to `g`:
                 */
                return fapply(g, fapply(f, f_in...));
            };
        },
        f, std::make_tuple(std::forward<G>(g)...),
        std::index_sequence<sizeof...(G)>{});
}

template <typename F, typename... G> auto fcompose_async(F f, G &&... g) {
    return fcompose(
        [](auto f, auto g) {
            return [ f = std::move(f), g = std::move(g) ](auto... f_in) {
                /*
                 * Assume that the convention is that we pass the callback as
                 * the last argument. With this convention, compose `f` and `g`
                 * by passing to `f` its arguments and a callback that takes
                 * in input the output of `f` and calls `g` passing to it the
                 * output of `f` as argument and as callback the callback passed
                 * as the last argument in `args...`.
                 */
                auto f_tuple = std::make_tuple(f_in...);
                auto f_rev = freverse(f_tuple);
                auto g_cb = fcar(f_rev);
                auto args = freverse(fcdr(f_rev));
                auto f_cb = std::make_tuple([
                    g = std::move(g), g_cb = std::move(g_cb)
                ](auto... f_out) { fapply(g, f_out..., g_cb); });
                fapply(f, std::tuple_cat(args, f_cb));
            };
        },
        f, std::make_tuple(std::forward<G>(g)...),
        std::index_sequence<sizeof...(G)>{});
}

} // namespace mk
#endif
