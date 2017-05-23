# NAME
ReturnType &mdash; Deduce return type of callable

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

/* Template magic */

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `ReturnType` template allows to deduce the return type of a callable; e.g.:

```C++
auto lambda = [](int x) -> double { return x * 1.7; };`
typename ReturnType<lambda>::type y = 6.28; // y would be a double
```

This can be useful to make more complex templates.

# HISTORY

The `ReturnType` template appeared in MeasurementKit 0.7.0.
