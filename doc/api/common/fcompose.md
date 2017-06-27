# NAME
fcompose &mdash; Compose arbitrary functors using different composing policies

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Policy, typename Functor, typename... OtherFunctors>
constexpr auto fcompose(Policy &&, Functor &&, OtherFunctors &&...);

class fcompose_policy_sync {
  public:
    fcompose_policy_sync();
};

class fcompose_policy_async {
  public:
    fcompose_policy_async();
};

}

```

# STABILITY

2 - Stable

# DESCRIPTION

The `fcompose` template function composes the functions passed as arguments
according to the policy specified as its first argument.

The `fcompose_policy_sync` policy is such that, given two functions `f`
and `g`, the f-composed-g function takes as arguments the arguments of `f`
and has the return value of `g`.

The `fcompose_policy_async` policy is that that, given two functions `f`
with arity `N` and `g` that have a callback as their last argument, the
f-composed-g function takes `N` arguments: the first `N - 1` arguments
of `f` followed by the callback of `g`.

# EXAMPLE

See `example/common/fcompose.cpp`.

# HISTORY

The `fcompose.hpp` header appeared in MeasurementKit 0.7.0.
