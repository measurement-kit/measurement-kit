// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_FCOMPOSE_HPP
#define PRIVATE_COMMON_FCOMPOSE_HPP

#include "private/common/fapply.hpp"
#include "private/common/fcar.hpp"
#include "private/common/fcdr.hpp"
#include "private/common/freverse.hpp"

namespace mk {

// pathologic corner case
template <typename P, typename F, typename... G>
constexpr auto fcompose_(const P &, F &&f, std::tuple<G...> &&,
                         std::index_sequence<0>) {
    return f;
}

// base case
template <typename P, typename F, typename... G>
constexpr auto fcompose_(const P &p, F &&f, std::tuple<G...> &&g,
                         std::index_sequence<1>) {
    return p(f, fcar(g));
}

// generic case
template <typename P, typename F, typename... G, std::size_t I,
          typename = typename std::enable_if<(I >= 2)>::type>
constexpr auto fcompose_(const P &p, F &&f, std::tuple<G...> &&g,
                         std::index_sequence<I>) {
    return fcompose_(p, p(f, fcar(g)), fcdr(std::move(g)),
                     std::index_sequence<I - 1>{});
}

// Set policy `p`, convert `g...` into a tuple and start compile time recursion
template <typename P, typename F, typename... G>
constexpr auto fcompose(P &&p, F &&f, G &&... g) {
    return fcompose_(p, f, std::make_tuple(std::forward<G>(g)...),
                     std::index_sequence<sizeof...(G)>{});
}

class fcompose_policy_sync {
  public:
    template <typename F, typename G>
    constexpr auto operator()(F &&f, G &&g) const {
        return [ f = std::move(f), g = std::move(g) ](auto &&... f_in) mutable {
            return fapply(g, fapply(f, std::move(f_in)...));
        };
    }
};

class fcompose_policy_async {
  public:
    template <typename F, typename G>
    constexpr auto operator()(F &&f, G &&g) const {
        return [ f = std::move(f), g = std::move(g) ](auto &&... f_in) mutable {
            auto f_tuple = std::make_tuple(std::move(f_in)...);
            auto f_rev = freverse(std::move(f_tuple));
            auto g_cb = fcar(f_rev);
            auto args = freverse(std::move(fcdr(std::move(f_rev))));
            auto f_cb = [ g = std::move(g),
                          g_cb = std::move(g_cb) ](auto &&... f_out) mutable {
                fapply_with_callback(std::move(g), std::move(g_cb),
                                     std::move(f_out)...);
            };
            fapply_with_callback(std::move(f), std::move(f_cb),
                                 std::move(args));
        };
    }
};

} // namespace mk
#endif
