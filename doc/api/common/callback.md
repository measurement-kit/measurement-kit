# NAME
Callback &mdash; Syntactic sugar for writing callbacks.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template<typename... T> using Callback<T...> = std::function<void(T...)>;

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `Callback` alias allows to more compactly writing callbacks and SHOULD be
used to indicate one-shot callbacks instead of `std::function<T>`.

# HISTORY

The `Callback` alias appeared in MeasurementKit 0.2.0.
