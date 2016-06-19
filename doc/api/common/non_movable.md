# NAME
Constraints -- Forbids movability

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class NonMovab;e {
  public:
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

}
```

# DESCRIPTION

Forbids movability. You should
forbid movability for example when under the hood the object's
`this` is passed to a C callback as an opaque pointer, thus binding
the specific value of `this` to the registered low-level callback.

# HISTORY

`NonCopyable` and `NonMovable` appeared in MeasurementKit 0.1.0.
