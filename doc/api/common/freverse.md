# NAME
freverse &mdash; Get the remainder of a tuple

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

assert(mk::freverse(std::make_tuple(1, 2, 3)) == std::make_tuple(3, 2, 1));
```

# STABILITY

2 - Stable

# DESCRIPTION

The `freverse` template function takes in input a tuple and returns
back the reverse of such tuple.

# HISTORY

The `freverse` template function appeared in MeasurementKit 0.7.0.
