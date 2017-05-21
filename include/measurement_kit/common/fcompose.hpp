// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP
#define MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP

#include <measurement_kit/common/fapply.hpp>
#include <measurement_kit/common/fcar.hpp>
#include <measurement_kit/common/fcdr.hpp>

namespace mk {

#if 0
// applies to `g` the return value of `f(args...)`
template <typename F, typename G> auto fcompose_impl_(F f, G g) {
    return [f = std::move(f), g = std::move(g)](auto... args) {
        /*
         * We do this in two steps such that, if `f` returns a tuple,
         * then the tuple is unpacked as the arguments to `g`.
         */
        return fapply(g, fapply(f, args...));
    };
}
#endif

/*
f: [](double, int, Callback<std::string>) {}

g: [](std::string, Callback<>) {}

f.g: [](double, int, Callback<>) {}
*/

template <typename F, typename G> auto fcompose_impl_(F f, G g) {
    // FIXME: it sucks that it's the first and not the last argument...
    return [ f = std::move(f), g = std::move(g) ](auto... f_in) {
        auto f_tuple = std::make_tuple(f_in...);
        auto cb = fcar(f_tuple);
        auto f_args = fcdr(f_tuple);
        /*
         * Note: here we basically need to construct a new tuple with all
         * arguments because this is what `fapply()` expects.
         */
        fapply(f, std::tuple_cat(
                      std::make_tuple([ g = std::move(g),
                                        cb = std::move(cb) ](auto... f_out) {
                          fapply(g, cb, f_out...);
                      }),
                      f_args));
    };
}

// pathologic corner case
template <typename F, typename... G>
auto fcompose(F f, std::tuple<G...>, std::index_sequence<0>) {
    return f;
}

// base case
template <typename F, typename... G>
auto fcompose(F f, std::tuple<G...> &&g, std::index_sequence<1>) {
    return fcompose_impl_(f, fcar(g));
}

// generic case
template <typename F, typename... G, std::size_t I,
          typename = typename std::enable_if<(I >= 2)>::type>
auto fcompose(F f, std::tuple<G...> &&g, std::index_sequence<I>) {
    return fcompose(fcompose_impl_(f, fcar(g)), fcdr(g),
                    std::index_sequence<I - 1>{});
}

// convert G... into a tuple and start compile-time recursion
template <typename F, typename... G> auto fcompose(F f, G &&... g) {
    // Note: here we should not call `make_index_sequence` because that
    // causes infinite compile time recursion. Instead what we want here
    // is to create the type bound to the current size of the sequence.
    return fcompose(f, std::make_tuple<G...>(std::forward<G>(g)...),
                    std::index_sequence<sizeof...(G)>{});
}

} // namespace mk
#endif
