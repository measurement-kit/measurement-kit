// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP
#define MEASUREMENT_KIT_COMMON_FCOMPOSE_HPP

#include <functional>

namespace mk {

/*
 * Note: with template magic it should be possible:
 *
 * 1. to make this a chain of callables so to avoid std::function
 *
 * 2. to extend this to an arbitrary number of callables
 *
 * Since this is a proof of concept, and I don't know yet how to
 * do 1. and 2., I've kept this simple for now.
 */
template <typename R, typename... A, typename G>
auto fcompose(std::function<R(A...)> f, G g) {
    return [=](A &&... a) { return g(f(std::forward<A>(a)...)); };
}

} // namespace mk
#endif
