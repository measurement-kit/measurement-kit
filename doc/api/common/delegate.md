# NAME
Delegate &mdash; Function that can modify itself when used as a method.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename T> class Delegate_ {
  public:
    Delegate_();
    template <typename F> Delegate_(F);
    Delegate_(std::function<void(T)>);

    void operator=(std::function<void(T)>);
    template <typename F> void operator=(F);
    void operator=(std::nullptr_t);

    operator bool();

    template <typename... A> void operator()(Args&& ...);
};

template <typename... T>
using Delegate = Delegate_<void(T...)>;

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `Delegate` template class is a `std::function<T>` like class that [can
change itself without triggering memory errors](https://github.com/measurement-kit/measurement-kit/issues/111).

The class is actually implemented using the `Delegate_` class that implements a
`std::function<>` with the possibility of changing itself and by the `Delegate` alias
that provides syntactic sugar for writing less when declaring delegates.

Constructors allow to construct the `Delegate`. The empty constructor creates an
empty function that raises `std::bad_function_call` if called. The templated
constructor allows to assign a lambda expression to this `Delegate`. Finally, the
function constructor allows to assign a `std::function<>` to this `Delegate`. Both
the templated and the function constructor shall fail at compile time if it is
not possible to initialize the `Delegate` using the lambda or the function.

The assignment operators allow to reassign the underlying function wrapped by
a `Delegate`. It is safe to call these operators to override the function that
the `Delegate` wraps from within the body of the delegate itself. The function
assignment operator overrides the underlying function with a newly specified
`std::function<>`. The templated assignment operator overrides the underlying
function with a lambda expression. The `nullptr` assignment operator resets
the underlying function; calling again the `Delegate` would then result into
a `std::bad_function_call` exception. Both the templated and the lambda assignments
would fail at compile time if types are not compatible.

The bool operator returns true if the underlying function is callable and false
if attempting to call the delegate would raise `std::bad_function_call`.

The call operator allows to call the `Delegate`.

# HISTORY

The `Delegate` template class appeared in MeasurementKit 0.2.0.
