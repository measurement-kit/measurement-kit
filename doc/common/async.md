# NAME
Async -- run tests asynchronously.

# LIBRARY
MeasurementKit (libmeasurement\_kit, -lmeasurement\_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>
#include <measurement_kit/foo.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit::foo;

Async *async = new Async;

void on_run_test() {
    SharedPointer<FooTest> test = std::make_shared<FooTest>(
        // Test configuration params
    );
    test->set_verbose(1);
    test->on_log([](const char *log_line) {
        // Caution: str points to a static buffer, make sure you copy
        // its content if you plan to use it at a later time
        // Caution: this callback is called from a background thread
    });
    async->run_test(test, [](SharedPointer<NetTest> test) {
        // Do something with the terminated test
        // Caution: this callback is called from a background thread
    });
}

// Tell async to break out of the event loop as soon as possible
// Note: this function returns immediately
async->break_loop();

// Wait for async to break out of the event loop
while (!async->empty()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

# DESCRIPTION

`Async` allows the App programmer to run MeasurementKit tests
in a background thread of execution.

The `Async` object itself should be allocated on the heap to guarantee
that it has the same lifecycle of the App.

To schedule a test to run asynchronously, do the following:

1. allocate the test object on the heap using `std::make_shared<T>`

2. (optional) tell the test to produce verbose messages using
   its `set_verbose` method

3. (optional) tell the test where you want its logs to be sent, by providing
   a log-line C++11 lambda. Be careful that the `const char *` pointer you
   receive is a pointer to a static buffer. Thus, make sure you copy the
   content over if you plan to use it later. Be careful that the log-line
   lambda will be called from a background thread context.

4. call `run_test` to schedule the test for execution, by providing a
   test-complete C++11 lambda. Be careful that the test-complete C++11
   lambda will be called from a background thread context.

If you need to quickly interrupt the I/O loop running in the background
thread, use `break_loop`. This method sets a flag telling the I/O loop to
break as soon as possible and then returns. Afterwards, periodically
call `empty()` to identify the moment in which no tests are running (and
hence the moment in which the background thread is stopped).

# HISTORY

The `Async` class appeared in MeasurementKit 0.1.
