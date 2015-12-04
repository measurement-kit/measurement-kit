# NAME
Libs -- Wrappers for C libraries functions.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

class Foo {
  public:
    // ...

  private:
    Libs *libs_ = mk::get_global_libs();
}

```


# DESCRIPTION

The `get_global_libs()` function allows to access the global `Libs`
objects.  This object is used internally to implement the regress
tests. But the implementation is opaque. We export only this function the
allow to access the default implementation.

# HISTORY

The `Libs` class appeared in MeasurementKit 0.1.
