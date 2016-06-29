# NAME
ErrorOr &mdash; Maybe-like object containing type or error.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename T> class ErrorOr {
  public:
    ErrorOr();
    ErrorOr(T);
    ErrorOr(Error);

    operator bool() const;

    const T &as_value() const;
    T &as_value();

    Error as_error() const;

    const T &operator*() const;
    T &operator*();

    const T *operator->() const;
    T *operator->();
};

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `ErrorOr` template class wraps either a type `T` or an `Error`. It is
commonly used in MeasurementKit to return an error in synchronous functions
as shown in the following snippet:

```C++
    ErrorOr<Result> res = get_result();
    if (!res) {
        callback(res.as_error());
        return;
    }
    use_result(*res);
```
The first form of the constructor creates an `ErrorOr` object initialized
with error equal to `NotInitializedError`. This constructor allows
to write code like the following:

```C++
    ErrorOr<Result> res;
    // ...
    
    /* Note that until assigned:
    
    REQUIRE_THROWS_AS(*res, NotInitializedError());
    REQUIRE(res.as_error() == NotInitializedError()); */
    
    // ...
    res = some_operation();
```

The second form of the constructor initializes the `ErrorOr` template
with a value. The third form of the constructor initialized the `ErrorOr`
template with an error.

The `operator bool()` method returns true if the underlying error object
has error code equal to zero, false otherwise. In the former case, we say
the `ErrorOr` contains a value, otherwise it contains an error.

The `as_value()` method returns a reference to the (possibly `const`)
underlying value, if the `ErrorOr` contains a value; otherwise, the
contained error is thrown.

The `as_error()` method returns the error field. Calling this method never
throws, regardless of whether the `ErrorOr` contains an error or
a value. This method is typically called to access the underlying error
once `operator bool()` has been used to ascertain that the `ErrorOr`
contains an error, as shown in the above snippet.

The `operator*()` method is an alias for `as_value()`.

The `operator->()` method returns a (possibly `const`) pointer to the
underlying field, if the `ErrorOr` contains a value; otherwise, the
contained error is thrown. This method could be useful to access fields
of a structure directly.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

using namespace mk;

class Result {
  public:
    unsigned int foo = 17;
    double bar = 3.14;
};

static ErrorOr<int> generate_seventeen() {
    return 17;
}

static ErrorOr<Result> generate_result() {
    return Result{};
}

static ErrorOr<Result> generate_error() {
    return MockedError();
}

int main() {
    ErrorOr<int> seventeen = generate_seventeen();
    REQUIRE(!!seventeen);
    REQUIRE(*seventeen == 17);
    *seventeen = 42;
    REQUIRE(*seventeen == 42);

    ErrorOr<Result> result = generate_result();
    REQUIRE(!!result);
    REQUIRE(result->foo == 17);
    REQUIRE(result->bar = 3.14);
    result->foo = 42;
    REQUIRE(result->foo == 42);

    ErrorOr<Result> error = generate_error();
    REQUIRE(!error);
    REQUIRE_THROWS(*error);
    REQUIRE_THROWS(error->foo);
    REQUIRE(error.as_error() == MockedError());
}
```

# HISTORY

The `ErrorOr` template class appeared in MeasurementKit 0.2.0. This template class
was originally called `Maybe` but it was later renamed `ErrorOr` because there is
[a namesake class with similar purpose in LLVM](http://llvm.org/docs/doxygen/html/classllvm_1_1ErrorOr.html).
