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

class fcompose_policy_async_robust {
  public:
    template <typename Errback> fcompose_policy_async_robust(Errback &&);
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

The `fcompose_policy_async_robust` function is like `fcompose_policy_async`
except that it routes exceptions occurring in `f` prior to calling `g` to
the callable passed as the first argument to the policy function itself.

# EXAMPLE

See `example/common/fcompose.cpp`.

# HISTORY

The `fcompose` template function appeared in MeasurementKit 0.7.0.
