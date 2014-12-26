/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_COMMON_POINTER_HPP
# define LIBIGHT_COMMON_POINTER_HPP

#include <memory>
#include <stdexcept>

namespace ight {
namespace common {
namespace pointer {

template<typename T> class SharedPointer : public std::shared_ptr<T> {
    using std::shared_ptr<T>::shared_ptr;

public:
    T *operator->() {
        if (this->get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator->();
    }

    typename std::add_lvalue_reference<T>::type operator*() {
        if (this->get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator*();
    }
};

}}}  // namespaces
#endif  // LIBIGHT_COMMON_POINTER_HPP
