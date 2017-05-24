# NAME
locked &mdash; Run callable with lock held

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename F> auto locked(std::mutex &mutex, F &&fun);

template <typename F> auto locked_global(F &&fun);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

Syntactic sugar templates that held the lock on some mutex and then call
`fun` with the lock held. Any exception will be propagated.

The `locked` template function uses a specific mutex. The `locked_global`
function uses a global mutex.

The return value of `fun`, if any, will be propagated. For example:

```C++
    int x = locked_global([]() { return 17; });
```

Both template functions take single ownership of the passed function.

# HISTORY

This family of template functions appeared in MeasurementKit 0.7.0.
