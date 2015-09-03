# NAME
Constraints -- Constraints on copyability and movability

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace measurement_kit {
namespace foobar {

class Foo : public common::NonCopyable {};

class Bar : public common::NonMovable {};

class FooBar : public common::NonCopyable,
               public common::NonMovable {};

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
