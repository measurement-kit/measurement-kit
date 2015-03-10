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

/*!
 * \brief Improved std::shared_ptr<T> with null pointer checks.
 *
 * This template class is a drop-in replacemente for the standard library's
 * shared_ptr<>. It extends shared_ptr<>'s ->() and *() operators to check
 * whether the pointee is a nullptr. In such case, unlike shared_ptr<>, the
 * pointee is not accessed and a runtime exception is raised.
 *
 * Use this class as follows:
 *
 *     using namespace ight::common::pointer;
 *     ...
 *     SharedPointer<Foo> ptr;
 *     ...
 *     ptr = std::make_shared<Foo>(...);
 *
 * That is, declare ptr as SharedPointer<Foo> and the behave like ptr was
 * a shared_ptr<> variable instead.
 *
 * It is safe to assign the return value of std::make_shared<Foo> to
 * SharedPointer<Foo> because SharedPointer have exactly the same fields
 * as std::shared_pointer and because it inherits the copy and move
 * constructors from std::shared_pointer.
 */
template<typename T> class SharedPointer : public std::shared_ptr<T> {
    using std::shared_ptr<T>::shared_ptr;

public:

    /*!
     * \brief Access the pointee to get one of its fields.
     * \returns A pointer to the pointee that allows one to access
     *          the requested pointee field.
     * \throws std::runtime_error if the pointee is nullptr.
     */
    T *operator->() {
        if (this->get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator->();
    }

    /*!
     * \brief Get the value of the pointee.
     * \returns The value of the pointee.
     * \throws std::runtime_error if the pointee is nullptr.
     */
    typename std::add_lvalue_reference<T>::type operator*() {
        if (this->get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return std::shared_ptr<T>::operator*();
    }
};

}}}  // namespaces
#endif  // LIBIGHT_COMMON_POINTER_HPP
