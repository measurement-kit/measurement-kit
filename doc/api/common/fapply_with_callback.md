# NAME
fapply_with_callback &mdash; Specialized fapply for functors with callback

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Functor, template Callback, typename... Args>
constexpr auto fapply_with_callback(
    Functor &&functor, Callback &&callback, Args &&... args);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fapply_with_callback` template function applies to functor passed
as its first argument the other arguments according as follows:

- The variadic list of arguments `args...` will be passed first

- The callback `callback` will be passed as last argument

If the functor returns a value, such value will be returned by the
`fapply_with_callback` template function as well.

In theory it would be possible to implement `fapply_with_callback` using
only `fapply`, by concatenating tuples. However, we have a number of cases
where we have callbacks and we want to save on concatenating tuples.

# EXAMPLE

See `example/common/fapply_with_callback.cpp`.

# HISTORY

The `fapply_with_callback` template function appeared in MeasurementKit 0.7.0.
