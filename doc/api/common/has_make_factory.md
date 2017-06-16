# NAME
HasMakeFactory &mdash; Decorator to add a factory to a class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template typename<T> class HasMakeFactory {
  public:
    template <typename... Args> static Var<T> make(Args &&... args);
};

}
```

# STABILITY

2 - Stable

# DESCRIPTION

Given class `T`, you can make `T` inherit from `HasMakeFactory<T>` to add
the implementation of a factory called `make()` to `T`.

The `make` factory will automatically allocate for you a new object of
type `T` every time you call it. It is syntactic sugar for:

```C++
    Var<Foo> foo = std::make_shared<Foo>(arguments...);
```

I introduced this syntactic sugar, in particular, for two reasons:

1. because it would be efficient to call `std::make_shared<T>` everywhere
   instead of doing something like

   ```C++
   Var<Foo> foo{new Foo};
   ```

   given that `make_shared` allocates both the control block of the `Var`
   and the pointee in a single memory allocation

2. but the current implementation of `Var` is fragile in that it depends onto
   the fact that we're not going to create object slicing, which is a promise
   that may be broken by refactoring the `Var` template

Hence, this `make` factory that forwards allocation to `Var::make()` is
the right choice because, if `Var` changes hopefully it will also change the
implementation of `Var::make` accordingly and we would not have the code
riddled with `std::make_shared` calls that create issues (or calls where we
call the allocator twice, once for the control block and once to allocate
the pointee).

# HISTORY

The `HasMakeFactory` template class appeared in MeasurementKit 0.7.0.
