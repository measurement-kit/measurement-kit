# NAME
gapply &mdash; Apply arguments to a functor

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::fapply([](int x, int y) { return x + y; }, 10, 7);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fapply` template function applies to functor passed as its first
argument all the following arguments and, if the functor returns a
result, returns such result.

# HISTORY

The `fapply` template function appeared in MeasurementKit 0.7.0.
