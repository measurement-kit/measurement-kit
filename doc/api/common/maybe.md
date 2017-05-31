# NAME
Maybe &mdash; The Maybe monad is back

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template typename<T> class Maybe {
  public:
    Maybe();
    Maybe(T &&t);
    T &operator*();
    operator bool() const;
};

} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `Maybe<T>` monad either contains an instance of `T` or is empty.

The first form of the constructor creates an empty monad.

The second form of the constructor creates a monad that owns the value `t`.

If you dereference a monad and the monad contains a value, such value is
returned. Otherwise a runtime exception is thrown.

To check whether the monad contains a value of not, evaluate it in a
boolean context. If the result of such evaluation is true, the monad
contains a type, otherwise it is empty.

# HISTORY

The `Maybe` template class was part of earlier version of MK and such
implementation was later renamed `ErrorOr`, because it was more similar
to an `ErrorOr` than to a proper `Maybe` monad. We have added back the
`Maybe` monad with its proper semantic in MK v0.7.0.
