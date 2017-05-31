# NAME
sandbox &mdash; Evaluate a callable in a protected context and filter exceptions

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Callable> Error sandbox_for_errors(Callable fun);

template <typename Callable>
Maybe<std::exception> sandbox_for_exceptions(Callable fun);

} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `sandbox_for_errors` template function will evaluate `fun` (which must
be equivalent to `std::function<void()>`) in a protected context. If the
evaluation of `fun` raises an `Error`, such error will be returned. Otherwise,
this template function will return `NoError()`.

The `sandbox_for_exceptions` template function is similar to the
`sandbox_for_errors` template, except that it catches any `std::exception`.
The return value is a `Maybe<std::exception>`. If no exception was raised,
said `Maybe` will be empty. Otherwise it will contain the exception.

Note that, since `Error` is a derived class of `std::exception`, the
`sandbox_for_exceptions` template function will by definition also trap
all kind of `Error`s. As such, this function is more general than
`sandbox_for_errors` is.

# HISTORY

The `common/sanbox` module was added in MK v0.7.0.
