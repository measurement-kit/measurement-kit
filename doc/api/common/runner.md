# NAME
Runner &mdash; run tests managing their lifecycle.

# LIBRARY
MeasurementKit (libmeasurement\_kit, -lmeasurement\_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class Runner {
  public:
    static Var<Runner> global();
    void run(Var<NetTest> test, Callback<Var<NetTest>> callback);
};

}
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `Runner` class runs tests in a background thread of executing
managing the lifecycle of tests. Typically, the code uses the global
`Runner`, accessible using the `global()` method.

The `run()` method receives a `Var<NetTest>` as its first argument and
a callback as its second argument. The test is scheduled for running
and the callback is called when the test is done.

Since the callback will be called from a background thread, make sure
you lock any shared resources before proceeding as in

```C++
    Runner::global()->run(test, [=](Var<NetTest>) {
        shared_resource.lock();
        // Now use the shared resource
    });
```

# HISTORY

The `Async` class appeared in MeasurementKit 0.1.0. It was renamed
`Runner` in MeasurementKit 0.2.0.
