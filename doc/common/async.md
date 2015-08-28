# NAME
Async -- run tests asynchronously.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>
#include <measurement_kit/foo.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::foo;

Async *async = new Async;

void on_run_test() {
    SharedPointer<FooTest> test = std::make_shared<FooTest>( /* params */ );
    async->run_test(test, [](NetTestPtr test) {
        // Do something with test...
    });
}

void periodically() { async->pump(); }
```

# DESCRIPTION

You should allocate an `Async` object on the heap, so that it has
the same lifecycle of your application. To schedule a test allocate
and configure it using a shared pointer, then pass such pointer to
`run_test()`. This will cause the test to run in a background thread
dedicated to running the event loop. Periodically call `pump()` so
that pending events (i.e. test-complete) are processed and the
corresponding callbacks passed to `run_test()` are called.

# HISTORY

The `Async` class appeared in MeasurementKit 0.1.
