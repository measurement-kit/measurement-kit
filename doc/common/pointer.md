# NAME
SharedPointer -- Smart shared pointer with nullptr checks

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

// Construct as a shared_ptr<T>

SharedPointer<T> p;                         // pointer is nullptr
SharedPointer<T> p(std::make_shared<T>());  // construct from shared_ptr
SharedPointer<T> p(new T());                // construct from raw pointer
p = std::make_shared<T>();                  // assign from shared_ptr

// The three overriden operations

T *rawptr = p.get();     // Get pointer value or raise
p->foo();                // Call T::foo() or raise
T value = *p;            // Return pointed value or raise
```

# DESCRIPTION

`SharedPointer<T>` template class is a drop-in replacement for the
standard library `std::shared_ptr<T>` template. It reimplements common
`std::shared_ptr<T>` operations by checking that the pointee is not
`nullptr`. Otherwise, a runtime exception is raised.

Whenever possible RAII should be used within MeasurementKit to guarantee
resources deallocation. In the few specific cases in which a pointer is
needed instead, a `SharedPointer<T>` should be used to guarantee that the
impact of programming errors (i.e. null pointers) is low.

It is safe to construct (or to assign) a `SharedPointer<T>` from an
`std::shared_ptr<T>`. In fact `SharedPointer<T>` is implemented overriding
the three basic operations of `std::shared_ptr<T>`.

# BUGS

Since `SharedPointer<T>` overrides `std::shared_ptr<T>` and since it is
meant to be used as a class (not as a pointer or as a reference), one should
remember not to add attributes to `SharedPointer<T>` implementation. This
guarantees that the following:

```C++
SharedPointer<T> p = std::make_shared<T>();
```

does not result in object slicing (i.e. in the construction of a
`SharedPointer<T>` with possibly uninitialized attributes).

# HISTORY

The `SharedPointer` class appeared in MeasurementKit 0.1.
