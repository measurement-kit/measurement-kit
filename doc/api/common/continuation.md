# NAME
Continuation &mdash; Syntactic sugar for functions that resume a paused function.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `Continuation` alias allows to more compactly express a function that
resumes another function that has paused waiting for explicit continuation.

# HISTORY

The `Continuation` alias appeared in MeasurementKit 0.2.0.
