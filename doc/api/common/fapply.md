# NAME
fapply &mdash; Apply arguments to a functor

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Functor, typename... Args>
constexpr auto fapply(Functor &&, Args &&...);

template <typename Functor, typename Callback, typename... Args>
constexpr auto fapply_with_callback(Functor &&, Callback &&, Args &&...);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fapply` template function applies to functor passed as its first
argument all the following arguments and, if the functor returns a
result, returns such result.

The `fapply_with_callback` template function is similar except that
it receives a callback separately. The callback will be passed as the
last argument to the functor. This is an optimization that avoid the
concatenation of tuples in `fcompose`.

# EXAMPLE

See `example/common/fapply.cpp`.

# HISTORY

The `fapply.hpp` header appeared in MeasurementKit 0.7.0.
