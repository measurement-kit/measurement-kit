# NAME
NonCopyable -- Forbids copyability

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class NonCopyable {
  public:
    NonCopyable(NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &) = delete;
};

}
```

# DESCRIPTION

Forbids copyability. You should in general
forbid copyability when you hold low-level pointers that should
be `free()`d just once.

To do this, just inherit from `NonCopyable` as in:

```C++
public class Foo : public mk::NonCopyable {
    // implementation here...
};
```


# HISTORY

`NonCopyable` appeared in MeasurementKit 0.1.0.
