# NAME
Callback -- Syntactic sugar for writing callbacks.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

template<typename... T> using Callback<T...> = std::function<void(T...)>;
```

# DESCRIPTION

The Callback alias allows to more compactly writing callbacks and SHOULD be
used to indicate one-shot callbacks instead of `std::function<T>`.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

void operation(Reactor r, Callback<Error> cb) {
    r.call_later(1.0, [=]() {
        cb(NoError());
    });
}
```

# STABILITY
2 - Stable

# HISTORY

The `Error` class appeared in MeasurementKit 0.2.0.
