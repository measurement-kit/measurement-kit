# NAME
every &mdash; call a functor every N seconds

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Callable, typename StopPredicate, typename Callback>
void every(double delay, Var<Reactor> reactor, Callback callback,
           StopPredicate stop_predicate, Callable callable);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `every` template function calls `callable` (equivalent to
`std::function<void()>`) every `delay` seconds using the specified `reactor`
as long as `stop_predicate` (equivalent to `std::function<bool()>`)
returns `false`. When `stop_predicate` returns `true`, the final `callback`
(equivalent to `std::function<void(Error)>`) will be called with a value
of `NoError`. In case `delay` is negative, `callback` will be called
with a value of `ValueError()`.

In no event `callback` will be called immediately. Even in case of error,
its execution will be scheduled as deferred with the `reactor`.

# HISTORY

The `every` template class appeared in MeasurementKit 0.7.0.
