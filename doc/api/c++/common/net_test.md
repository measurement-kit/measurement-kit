# NAME
NetTest -- Base network test class

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::NetTest *test = new foo::SomeNetTest();  // NetTest is abstract

// Configure the logger contained by NetTest
test->on_log([](const char *s) {
    // Do something
});
test->set_verbose(1);

// Get unique identifier of this test
unsigned long long identifier = test->identifier();

// Start test, tell to store results when done, tell to do something
// else when the results have been saved
test->begin([test]() {
    test->end([test]() {
        something_else(test);
    });
});
```

# DESCRIPTION

`NetTest` is the abstract base class of all tests. It defines the
basic behavior of a test. It is the "thing" you pass to the code for
running tests in a background thread (i.e. `Async`).

# BUGS

Since `Async` do not contain a mechanism for storing logs in a thread
safe way, you typically use `on_log()` to get logs and store them somewhere
within your app. Beware that the function you set with `on_log()` will be
called from the `Async` background thread context. You are responsbile
of doing the locking for yourself. (This may as well be a bug of `Async`
but mentioning it here to warn people using this interface.)

# HISTORY

The `NetTest` class appeared in MeasurementKit 0.1.
