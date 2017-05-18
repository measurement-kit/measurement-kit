# NAME
range &mdash; generates numbers from 0 to N

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename T> std::vector<T> range(size_t count);

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `range` function generates a vector containing all numbers
from zero to `count`.

# HISTORY

The `range` function appeared in MeasurementKit 0.3.0.
