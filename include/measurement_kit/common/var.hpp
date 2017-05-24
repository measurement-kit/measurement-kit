// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_VAR_HPP
#define MEASUREMENT_KIT_COMMON_VAR_HPP

#include <measurement_kit/common/ptr_.hpp>

namespace mk {

MK_DEFINE_PTR_(Var_, shared_ptr, make_shared);

template <typename T> class Var : public Var_<T> {
  public:
    using Var_<T>::Var_;
    template <typename R> Var<R> as() {
        return std::dynamic_pointer_cast<R>(*this);
    }
};

} // namespace mk
#endif
