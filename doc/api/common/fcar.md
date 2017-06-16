# NAME
fcar &mdash; Get first argument of a tuple

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

assert(mk::fcar(std::make_tuple(1, 2, 3)) == 1);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fcar` template function takes in input a non-empty tuple and
returns back the first element of the tuple.

The `fcar` template SHOULD NOT compile if the tuple is empty.

# HISTORY

The `fcar` template function appeared in MeasurementKit 0.7.0.
