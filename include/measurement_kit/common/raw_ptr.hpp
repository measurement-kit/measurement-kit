// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_RAW_PTR_HPP
#define MEASUREMENT_KIT_COMMON_RAW_PTR_HPP

#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <stdexcept>
#include <type_traits>

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
// This class is NonCopyable and NonMovable because it manages a C pointer. It
// should be wrapped in some other class that is movable or copyable through the
// usage of smart pointers.
template <typename Type, typename TypeDeleter>
class RawPtr : public NonCopyable, public NonMovable {
  public:
    // The get() method returns the underlying pointer, if that is not null, or
    // throws an exception otherwise. This provides the guarantee that we are
    // not going to access a null pointer after some bad refactoring.
    typename std::add_pointer<Type>::type get() const {
        if (ptr_ == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_;
    }

    // The cast-to-pointer-type operator is syntactic sugar for get().
    operator typename std::add_pointer<Type>::type() const { return get(); }

    // The release() method returns the underlying pointer and replaces the
    // underlying pointer with nullptr. This is the way to extract the pointer
    // from this wrapper without having the TypeDeleter invoked.
    typename std::add_pointer<Type>::type release() {
        typename std::add_pointer<Type>::type p = nullptr;
        std::swap(ptr_, p);
        return p;
    }

    // The reset() method will call the type deleter for the underlying pointer
    // if such pointer is not null, and then will replace the underlying
    // pointer with the provided parameter.
    void reset(typename std::add_pointer<Type>::type p = nullptr) {
        if (ptr_ != nullptr) {
            TypeDeleter{}(ptr_);
        }
        ptr_ = p;
    }

    // The constructor with pointer takes ownership of the pointer argument.
    RawPtr(typename std::add_pointer<Type>::type p) { ptr_ = p; }

    // The default constructor constructs an empty pointer.
    RawPtr() {}

    // The destructor calls reset(nullptr) to invoke the TypeDeleter.
    ~RawPtr() { reset(nullptr); }

  private:
    typename std::add_pointer<Type>::type ptr_ = nullptr;
};

} // namespace mk
#endif
