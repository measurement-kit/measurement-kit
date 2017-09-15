# NAME

`measurement_kit/common/error_or.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```
#ifndef MEASUREMENT_KIT_COMMON_ERROR_OR_HPP
#define MEASUREMENT_KIT_COMMON_ERROR_OR_HPP

namespace mk {

template <typename T> class ErrorOr {
  public:
    ErrorOr() : error_(NotInitializedError()) {}

    ErrorOr(T value) : value_(value) {}

    ErrorOr(Error error) : error_(error) {}

    ErrorOr(Error error, T value) : error_(error), value_(value) {}

    operator bool() const { return error_ == NoError(); }

#define XX                                                                     \
    if (error_ != 0) {                                                         \
        throw error_;                                                          \
    }                                                                          \
    return value_;

    const T &as_value() const {XX}

    T &as_value() {
        XX
    }

#undef XX

    Error as_error() const { return error_; }

    const T &operator*() const { return as_value(); }

    T &operator*() { return as_value(); }

#define XX                                                                     \
    if (error_ != 0) {                                                         \
        throw error_;                                                          \
    }                                                                          \
    return &value_;

    const T *operator->() const {XX} T *operator->() { XX }

#undef XX

  private:
    Error error_;
    T value_;
};

} // namespace mk
#endif
```

# DESCRIPTION

`ErrorOr` wraps either a type or an `Error`. We often use it in synchronous code when we don't want to throw an exception. 

The following pattern is frequently used in MK: 

```C++ ErrorOr<Result> result = get_result_noexcept(); if (!result) { callback(result.as_error()); return; } use_result(*result); ``` 

Attempting to access the value (for example using as_value()) when ErrorOr wraps an error causes such error to be thrown. 

ErrorOr firstly appeared in measurement-kit v0.2.0. originally named Maybe. We changed the name later because we have seen that also LLVM has a class called ErrorOr. 

We may probably want to get rid of ErrorOr when we switch to C++17 where unpacking of returned tuple is made pleasant.

The default constuctor creates a non-initialized ErrorOr. This means basically constructing it with an error and no value.

The constructor with value sets the underlying value.

The constructor with error sets the underlying error.

The constructor with error and value sets both.

Operator bool returns true if ErrorOr wraps a value.

`as_value()` returns the value if the ErrorOr contains a value and throws an error if ErrorOr contains an error.

`as_error()` returns the contained error or NoError().

Operator `*` is a compact way to call as_value().

Operator `->` allows to access the underlying value as a pointer, or throws an error if ErrorOr wraps an error.

