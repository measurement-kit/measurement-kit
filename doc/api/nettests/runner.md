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
    void Runner::start_generic_task(std::string &&name, Var<Logger> logger,
                                    Continuation<> &&task, Callback<> &&done);
};

}}
```

# STABILITY

1 - Experimental

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

The `start_generic_task` schedules the task identified by `name` for
execution in a background thread. The provided `logger` will be used to
emit diagnostic messages. The `task` argument is a continuation that
implements the task: it receives as a single argument a `Callback<>` that
is to be called when the task has finished its work. The code will make
sure to call the final callback, `done`, when this happens.

# HISTORY

The `Async` class appeared in MeasurementKit 0.1.0. It was renamed
`Runner` in MeasurementKit 0.2.0. It was moved from the `common` to
the `nettests` namespace in MeasurementKit 0.4.0. Support for running
arbitrary lambdas was added in v0.7.0.
