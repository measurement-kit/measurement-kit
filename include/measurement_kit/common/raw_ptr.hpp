// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_RAW_PTR_HPP
#define MEASUREMENT_KIT_COMMON_RAW_PTR_HPP

#include <memory>      // for std::unique_ptr
#include <stdexcept>   // for std::runtime_error
#include <type_traits> // for std::add_pointer_type

namespace mk {

// # RawPtr
//
// RawPtr is a template class depending on a Type and on a TypeDeleter that
// is used to provide RAII semantics for raw pointers. We will only invoke the
// type deleter when the underlying pointer is not null.
//
// In designing this class, we have used the same method names of existing
// libc++ smart pointers to provide the least surprise. However:
//
// 1. We have added a method to automatically cast the content of this class
//    to the underlying pointer to make the code easier to migrate.
//
// This class is implemented using std::unique_ptr as the underlying smart
// pointer type. This is becaused such smart pointer was the more similar one
// to the interface we wanted to implement. As a result, this makes code
// using this template class non-copyable but movable.
template <typename Type, typename TypeDeleter = std::default_delete<Type>>
class RawPtr {
  public:
    // The get() method returns the underlying pointer, if that is not null, or
    // throws an exception otherwise. This provides the guarantee that we are
    // not going to access a null pointer after some bad refactoring.
    typename std::add_pointer<Type>::type get() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.get();
    }

    // The cast-to-pointer-type operator is syntactic sugar for get().
    operator typename std::add_pointer<Type>::type() const { return get(); }

    // The release() method returns the underlying pointer and replaces the
    // underlying pointer with nullptr. This is the way to extract the pointer
    // from this wrapper without having the TypeDeleter invoked.
    typename std::add_pointer<Type>::type release() { return ptr_.release(); }

    // The reset() method will call the type deleter for the underlying pointer
    // if such pointer is not null, and then will replace the underlying
    // pointer with the `p` pointer parameter.
    void reset(typename std::add_pointer<Type>::type p = nullptr) {
        ptr_.reset(p);
    }

    // The constructor with pointer takes ownership of the pointer argument.
    RawPtr(typename std::add_pointer<Type>::type p) : ptr_{p} {}

    // The default constructor constructs an empty pointer.
    RawPtr() {}

  private:
    std::unique_ptr<Type, TypeDeleter> ptr_;
};

} // namespace mk
#endif
