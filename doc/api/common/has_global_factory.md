# NAME
HasGlobalFactory &mdash; Decorator to add global factory to a class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template typename<T> class HasGlobalFactory {
  public:
    template <typename... Args> static Var<T> global(Args &&... args);
};

}
```

# STABILITY

2 - Stable

# DESCRIPTION

Given class `T`, you can make `T` inherit from `HasGlobalFactory<T>` to add
the implementation of a global factory called `global()` to `T`.

The `Var<T>` returned by `global` will be a singleton.

# HISTORY

The `HasGlobalFactory` template class appeared in MeasurementKit 0.7.0.
