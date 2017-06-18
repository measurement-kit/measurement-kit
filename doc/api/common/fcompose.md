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
the callable passed as the first argument to the policy function itself with
type `const std::exception &`.

# CAVEATS

The `fcompose_policy_async_robust` policy will not catch any exception
raised by `g` or after `g` has been called. When composing an arbitrary
number of functions, the fact that we do not catch exceptions raised
by `g` implies that we do not catch exceptions raised by the final callback
in the chain. Which makes sense, because this final callback is outside of
the chain of functions. Thus, exceptions occurring in there should not be
handled as part of the chain of functions.

We also do want to provide the guarantee that *either* the exception handler
function is called *or* the final callback is called. After `g` has been
called, however, we don't have a way to know whether the sequence of calls
lead to the final callback or not. Thus, we will not catch exceptions
occurring after control has been passed from `f` to `g`. In practical terms,
this means your code MUST NOT do anything after it has transferred control
to the next callback. This is already the case in most MK code.

# EXAMPLE

See `example/common/fcompose.cpp`.

# HISTORY

The `fcompose` template function appeared in MeasurementKit 0.7.0.
