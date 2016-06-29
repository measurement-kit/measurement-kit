# NAME
NonMovable &mdash; Forbids movability

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class NonMovable {
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
To do this, just inherit from `NonMovable` as in:

```C++
/* Setting the class as non movable is crucial here because the
   constructor passes `this` to a low level API which would presumably
   use it to call `low_level_notification()`. Therefore, we should
   not allow to move this object, because moving implies destroying
   the previous object and moving its content to a new one. This would
   invalidate the `this` pointer passed to the low level API and thus
   after the move code of a delected object would be invoked. */
public class Foo : public mk::NonMovable {
  public:
    Foo() {
      low_level_api(this);
    }

    void low_level_notification() { /* do something */ }
};
```

# HISTORY

`NonMovable` appeared in MeasurementKit 0.1.0.
