// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_POINTER_HPP
#define MEASUREMENT_KIT_COMMON_POINTER_HPP

#include <memory>
#include <stdexcept>

namespace measurement_kit {
namespace common {

/// Improved std::shared_ptr<T> with null pointer checks
template <typename T> class SharedPointer : public std::shared_ptr<T> {
    using std::shared_ptr<T>::shared_ptr;

  public:
    T *get() const { return operator->(); } ///< Get the raw pointer

    /// Syntactic sugar to get the raw pointer
    T *operator->() const {
        if (std::shared_ptr<T>::get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator->();
    }

    /// Dereference the raw pointer
    typename std::add_lvalue_reference<T>::type operator*() const {
        if (std::shared_ptr<T>::get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator*();
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace common
} // namespace measurement_kit
#endif
