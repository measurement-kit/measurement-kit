# NAME
Runner &mdash; run tests managing their lifecycle.

# LIBRARY
MeasurementKit (libmeasurement\_kit, -lmeasurement\_kit).

# SYNOPSIS
```C++
#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

class Runner {
  public:
    static Var<Runner> global();
    void start_test(Var<NetTest> test, Callback<Var<NetTest>> callback);
    void stop();
    bool empty();
};

} // namespace nettests
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `Runner` class runs tests in a background thread of executing
managing the lifecycle of tests. Typically, the code uses the global
`Runner`, accessible using the `global()` method.

The `start_test()` method receives a `Var<NetTest>` as its first argument and
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

The `stop` method stops the runner I/O loop and terminates all the tests
that are currently running, without calling their callbacks.

The `empty` method returns true if no tests are running.

# HISTORY

The `Async` class appeared in MeasurementKit 0.1.0. It was renamed
`Runner` in MeasurementKit 0.2.0. It was moved from the `common` to
the `nettests` namespace in MeasurementKit 0.4.0.
