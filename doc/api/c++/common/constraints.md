# NAME
Constraints -- Constraints on copyability and movability

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {
namespace foobar {

class Foo : public NonCopyable {};

class Bar : public NonMovable {};

class FooBar : public NonCopyable, public NonMovable {};

}}
```

# DESCRIPTION

Constraints are used to restrict copyability and/or movability of
classes. You should in general forbid copyability when you hold
low-level pointers that should be `free()`d just once. You should
forbid movability for example when under the hood the object's
`this` is passed to a C callback as an opaque pointer, thus binding
the specific value of `this` to the registered low-level callback.

# HISTORY

`NonCopyable` and `NonMovable` appeared in MeasurementKit 0.1.
