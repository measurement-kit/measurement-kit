// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP
#define MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/fapply.hpp>
#include <measurement_kit/common/fcar.hpp>
#include <measurement_kit/common/fcdr.hpp>
#include <measurement_kit/common/freverse.hpp>

#include <exception>

namespace mk {

// pathologic corner case
template <typename P, typename F, typename... G>
auto fcompose_(const P &, F &&f, std::tuple<G...> &&, std::index_sequence<0>) {
    return f;
}

// base case
template <typename P, typename F, typename... G>
auto fcompose_(const P &p, F &&f, std::tuple<G...> &&g,
               std::index_sequence<1>) {
    return p(f, fcar(g));
}

// generic case
template <typename P, typename F, typename... G, std::size_t I,
          typename = typename std::enable_if<(I >= 2)>::type>
auto fcompose_(const P &p, F &&f, std::tuple<G...> &&g,
               std::index_sequence<I>) {
    return fcompose_(p, p(f, fcar(g)), fcdr(g), std::index_sequence<I - 1>{});
}

// Set policy `p`, convert `g...` into a tuple and start compile time recursion
template <typename P, typename F, typename... G>
constexpr auto fcompose(P &&p, F &&f, G &&... g) {
    return fcompose_(p, f, std::make_tuple(std::forward<G>(g)...),
                     std::index_sequence<sizeof...(G)>{});
}

// g(f(f_in...))
class fcompose_policy_sync {
  public:
    template <typename F, typename G>
    constexpr auto operator()(F &&f, G &&g) const {
        return [ f = std::move(f), g = std::move(g) ](auto &&... f_in) {
            return fapply(g, fapply(f, f_in...));
        };
    }
};

// g(f(f_in...[:-1]), f_in...[-1])
class fcompose_policy_async {
  public:
    template <typename F, typename G>
    constexpr auto operator()(F &&f, G &&g) const {
        return [ f = std::move(f), g = std::move(g) ](auto &&... f_in) {
            auto f_tuple = std::make_tuple(f_in...);
            auto f_rev = freverse(f_tuple);
            auto g_cb = fcar(f_rev);
            auto args = freverse(fcdr(f_rev));
            auto f_cb =
                  std::make_tuple([ g = std::move(g), g_cb = std::move(g_cb) ](
                        auto &&... f_out) { fapply(g, f_out..., g_cb); });
            fapply(f, std::tuple_cat(args, f_cb));
        };
    }
};

// Like above but also routes exceptions in `f`. We MUST NOT route exceptions
// in `g` because, from what we know, it's the final callback.
class fcompose_policy_async_robust {
  public:
    template <typename E> fcompose_policy_async_robust(E &&e) : errback_(e) {}

    template <typename F, typename G> auto operator()(F &&f, G &&g) const {
        // Make sure we don't pass `this` to the lambda because the lifetime
        // of this object is actually very ephemeral (see above)
        return [ f = std::move(f), g = std::move(g),
                 eb = errback_ ](auto &&... f_in) {
            auto f_tuple = std::make_tuple(f_in...);
            auto f_rev = freverse(f_tuple);
            auto g_cb = fcar(f_rev);
            auto args = freverse(fcdr(f_rev));
            auto e_route = true;
            auto f_cb = std::make_tuple(
                  [ g = std::move(g), g_cb = std::move(g_cb),
                    &e_route ](auto &&... f_out) {
                      e_route = false;
                      fapply(g, f_out..., g_cb);
                      e_route = true;
                  });
            try {
                fapply(f, std::tuple_cat(args, f_cb));
            } catch (const std::exception &exc) {
                if (!e_route) {
                    throw;
                };
                eb(exc);
            }
        };
    }

  private:
    Callback<const std::exception &> errback_;
};

} // namespace mk
#endif
