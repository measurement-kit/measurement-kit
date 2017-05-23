# NAME
locked &mdash; Run callable with lock held

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename R> R locked(std::mutex &mutex, std::function<R()> fun);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

Syntactic sugar that helds the lock on `mutex` and then calls `fun` with
the lock held. Any exception will be propagated.

# HISTORY

The `locked` template function appeared in MeasurementKit 0.7.0.
