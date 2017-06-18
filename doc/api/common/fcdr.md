# NAME
fcdr &mdash; Get from second to last element of a tuple

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

assert(mk::fcdr(std::make_tuple(1, 2, 3)) == std::make_tuple(2, 3));
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fcdr` template function takes in input a non-empty tuple, removes
the first element, and returns the rest of the tuple.

The `fcdr` template SHOULD NOT compile if the tuple is empty.

# HISTORY

The `fcdr` template function appeared in MeasurementKit 0.7.0.
