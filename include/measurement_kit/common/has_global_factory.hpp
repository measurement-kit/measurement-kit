// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_HAS_GLOBAL_FACTORY_HPP
#define MEASUREMENT_KIT_COMMON_HAS_GLOBAL_FACTORY_HPP

#include <measurement_kit/common/locked.hpp>
#include <measurement_kit/common/var.hpp>

namespace mk {

template <typename T> class HasGlobalFactory {
  public:
    template <typename... A> static Var<T> global(A &&... a) {
        return locked_global([&]() {
            static Var<T> singleton;
            if (!singleton) {
                singleton = Var<T>::make(std::forward<A>(a)...);
            }
            return singleton;
        });
    }
};

} // namespace mk
#endif
