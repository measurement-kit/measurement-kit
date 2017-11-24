// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_UNIQUE_PTR_HPP
#define MEASUREMENT_KIT_COMMON_UNIQUE_PTR_HPP

#include <memory>      // for std::unique_ptr
#include <stdexcept>   // for std::runtime_error
#include <type_traits> // for std::add_pointer_type

namespace mk {

/// # UniquePtr
///
/// UniquePtr is a wrapper for `std::unique_ptr` where accessing a null
/// pointer will throw a `std::runtime_error` exception.
template <typename Type, typename TypeDeleter = std::default_delete<Type>>
class UniquePtr {
  public:
    /// The get() method returns the underlying pointer, if that is not null, or
    /// throws an exception otherwise. This provides the guarantee that we are
    /// not going to access a null pointer after some bad refactoring.
    typename std::add_pointer<Type>::type get() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.get();
    }

    /// The star operator returns a reference to the underlying structure.
    typename std::add_lvalue_reference<Type>::type operator*() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.operator*();
    }

    /// The arrow operator is an alias for get().
    typename std::add_pointer<Type>::type operator->() const { return get(); }

    /// The release() method returns the underlying pointer and replaces the
    /// underlying pointer with nullptr. This is the way to extract the pointer
    /// from this wrapper without having the TypeDeleter invoked.
    typename std::add_pointer<Type>::type release() { return ptr_.release(); }

    /// The reset() method will call the type deleter for the underlying pointer
    /// if such pointer is not null, and then will replace the underlying
    /// pointer with the `p` pointer parameter.
    void reset(typename std::add_pointer<Type>::type p = nullptr) {
        ptr_.reset(p);
    }

    /// The cast-to-bool operator returns true if the underlying pointer
    /// is not nullptr and false otherwise.
    operator bool() const { return static_cast<bool>(ptr_); }

    /// The constructor with pointer takes ownership of the pointer argument.
    explicit UniquePtr(typename std::add_pointer<Type>::type p) : ptr_{p} {}

    /// The default constructor constructs an empty pointer.
    UniquePtr() {}

  private:
    std::unique_ptr<Type, TypeDeleter> ptr_;
};

} // namespace mk
#endif
