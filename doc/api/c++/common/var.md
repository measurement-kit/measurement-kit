# NAME
Var -- Shared-pointer with JavaScript-var-like semantic

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

// Construct as a shared_ptr<T>

mk::Var<T> p;                         // pointer is nullptr
mk::Var<T> p(new T());                // construct from raw pointer

// The three overriden operations

T *rawptr = p.get();     // Get pointer value or raise
p->foo();                // Call T::foo() or raise
T value = *p;            // Return pointed value or raise
```

# DESCRIPTION

`Var<T>` template class is a drop-in replacement for the
standard library `std::shared_ptr<T>` template. It reimplements common
`std::shared_ptr<T>` operations by checking that the pointee is not
`nullptr`. Otherwise, a runtime exception is raised.

Whenever possible RAII should be used within MeasurementKit to guarantee
resources deallocation. In the few specific cases in which a pointer is
needed instead, a `Var<T>` should be used to guarantee that the
impact of programming errors (i.e. null pointers) is low.

It is safe to construct (or to assign) a `Var<T>` from an
`std::shared_ptr<T>`. In fact `Var<T>` is implemented overriding
the three basic operations of `std::shared_ptr<T>`.

# BUGS

Since `Var<T>` overrides `std::shared_ptr<T>` and since it is
meant to be used as a class (not as a pointer or as a reference), one should
remember not to add attributes to `Var<T>` implementation. This
guarantees that the following:

```C++
mk::Var<T> p = std::make_shared<T>();
```

does not result in object slicing (i.e. in the construction of a
`Var<T>` with possibly uninitialized attributes).

# HISTORY

The `Var` class appeared in MeasurementKit 0.1.0.
