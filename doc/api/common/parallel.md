# NAME
Parallel &mdash; allows running Continuations in parallel

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

void parallel(srd::vector<Continuation<Error>> input, Callback<Error> cb);

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `parallel` function runs all the continuations in the input vector in
parallel and invokes the final callback when all the continuations have
terminated their job (i.e. each continuation's callback was called).

The overall error passed to the callback `cb` is `NoError` only if all the
continuation's callbacks succeeded, `ParallelOperationError` otherwise. Check
the `child_errors` field of the `Error` passed to `cb` to know whether each
continuation failed (and why) or succeeded.

# BUGS

The `parallel` implementation assumes that all the continuations will run
in the same thread, therefore the state used to decide when all continuations
have completed is not thread safe.

# HISTORY

The `parallel` function appeared in MeasurementKit 0.3.0.
