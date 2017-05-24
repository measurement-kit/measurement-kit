# NAME
U &mdash; Unique-pointer with null pointer check

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template typename<T> class U : public std::unique_ptr<T> {
  public:
    U(T ptr);
    U();
    void reset(T *ptr);
    T *get();
    T *operator->() const;
    T &operator*() const;
    static template <typename... Args> U<T> make(Args &&... args);
};

}
```

# STABILITY

2 - Stable

# DESCRIPTION

`U<T>` template class is a drop-in replacement for the
standard library `std::unique_ptr<T>` template. It reimplements common
`std::unique_ptr<T>` operations by checking that the pointee is not
`nullptr`. Otherwise, a runtime exception is raised.

The first form of the constructor *owns* `ptr` and manages its life
cycle. The second form of the constructor initializes to `nullptr` the
internal pointer; attempting to dereference a `U<>` initialized
by this form of the constructor raises a `std::runtime_error`.

The `reset()` method releases the previously pointed object and then
*owns* the object pointed by `ptr` and manages its life cycle. It is legal
to pass `nullptr` to this function; in such case further attempts to
access the pointee would result in `std::runtime_error` being raised.

The `get()` and `operator->()` methods return the pointee if non null and
throw `std::runtime_error` otherwise.

The `operator*()` method returns a reference to `*ptr` where `ptr` is the
pointee if the pointee is non null and throws `std::runtime_error` otherwise.

The static `make()` method is equivalent to:

```C++
U<T> t{new T{arguments...}};
```

except that, like for `std::make_unique`, only one allocation is performed
to allocate both the unique pointer control block and the pointee.

# HISTORY

The `U` template class appeared in MeasurementKit 0.7.0.
