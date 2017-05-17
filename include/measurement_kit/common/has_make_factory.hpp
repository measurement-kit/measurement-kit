// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_HAS_MAKE_FACTORY_HPP
#define MEASUREMENT_KIT_COMMON_HAS_MAKE_FACTORY_HPP

#include <measurement_kit/common/var.hpp>

namespace mk {

template <typename T> class HasMakeFactory {
  public:
    template <typename... A> static Var<T> make(A &&... a) {
        return Var<T>::make(std::forward<A>(a)...);
    }
};

} // namespace mk
#endif
