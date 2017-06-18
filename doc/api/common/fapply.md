# NAME
fapply &mdash; Apply arguments to a functor

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Functor, typename... Args>
constexpr auto fapply(Functor &&functor, Args &&... args);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fapply` template function applies to functor passed as its first
argument all the following arguments and, if the functor returns a
result, returns such result.

# EXAMPLE

See `example/common/fapply.cpp`.

# HISTORY

The `fapply` template function appeared in MeasurementKit 0.7.0.
