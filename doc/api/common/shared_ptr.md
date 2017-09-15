# NAME

`measurement_kit/common/shared_ptr.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_SHARED_PTR_HPP
#define MEASUREMENT_KIT_COMMON_SHARED_PTR_HPP

namespace mk {

template <typename T> class SharedPtr {
  public:
    template <typename Deleter>
    SharedPtr(typename std::add_pointer<T>::type p, Deleter &&deleter)
        : ptr_{std::shared_ptr<T>{p, deleter}} {}

    SharedPtr(typename std::add_pointer<T>::type p)
        : ptr_{std::shared_ptr<T>{p}} {}

    SharedPtr(std::shared_ptr<T> &&ptr) : ptr_{ptr} {}

    SharedPtr() {}

    long use_count() const noexcept { return ptr_.use_count(); }

    operator bool() const { return static_cast<bool>(ptr_); }

    void reset(typename std::add_pointer<T>::type p = nullptr) {
        ptr_.reset(p);
    }

    bool operator==(std::nullptr_t x) const noexcept { return ptr_ == x; }

    template <typename Deleter>
    void reset(typename std::add_pointer<T>::type p, Deleter &&deleter) {
        ptr_.reset(p, deleter);
    }

    typename std::add_pointer<T>::type get() const { return operator->(); }

    typename std::add_pointer<T>::type operator->() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.operator->();
    }

    typename std::add_lvalue_reference<T>::type operator*() const {
        if (ptr_.get() == nullptr) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.operator*();
    }

    template <typename R> SharedPtr<R> as() const {
        return std::dynamic_pointer_cast<R>(ptr_);
    }

    template <typename... A> static SharedPtr<T> make(A &&... a) {
        return std::make_shared<T>(std::forward<A>(a)...);
    }

  private:
    std::shared_ptr<T> ptr_;
};

} // namespace mk
#endif
```

# DESCRIPTION

`SharedPtr` is a null-safe wrapper for std::shared_ptr. The idea is that we'd rather throw than crash if attempting to access a null pointer. 

You should be able to use SharedPtr like you use std::shared_ptr. In particular, you can use std::make_shared to construct objects and then you can store the result of std::make_shared into a SharedPtr. 

Appeared in measurement-kit v0.1.0. 

Before MK v0.8.0, SharedPtr was named Var.

`as` allows to convert SharedPtr<T> into SharedPtr<R> just like std::dynamic_pointer_cast does for std::shared_ptr. Returns a SharedPtr<R> which will wrap `nullptr` and hence throw when accessed if the dynamic cast failed. Otherwise, you can of course safely used the returned SharedPtr.

`make()` is an alternative way of constructing a SharedPtr. This factory was added before it was possible and generally safe to assign the return value of std::make_shared to a SharedPtr. After MK v0.8.0. it is reccommended to use std::make_shared rather than make(). We may deprecate make() in a future version of MK.

