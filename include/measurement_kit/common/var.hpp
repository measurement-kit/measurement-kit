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
    typename std::add_pointer<T>::type get() const { return operator->(); }

    typename std::add_pointer<T>::type operator->() const {
        if (std::shared_ptr<T>::get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator->();
    }

    typename std::add_lvalue_reference<T>::type operator*() const {
        if (std::shared_ptr<T>::get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator*();
    }

    template <typename R> Var<R> as() {
        return std::dynamic_pointer_cast<R>(*this);
    }

    template <typename... A> static Var<T> make(A &&... a) {
        return std::make_shared<T>(std::forward<A>(a)...);
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
