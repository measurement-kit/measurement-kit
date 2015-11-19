# NAME
Libs -- Wrappers for C libraries functions.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

// Suppose you want to check whether the Poller constructor deals
// well with `event_base_new()` returning `nullptr`, then:

Libs libs;
libs.event_base_new = []() { return nullptr; };
bool exc = false;
try {
    Poller poller(&libs);
} catch (std::bad_alloc &) {
    exc = true;
}
if (!exc) { /* test failed, do something */ }
```

# DESCRIPTION

The `Libs` object contains C++11 lambdas mocking the APIs of many
low-level APIs used by MeasurementKit. For example, `Libs` contains
a C++11 lambda mocking `event_base_new()`.

By default, those lambdas are initialized to the corresponding APIs
such that, when you call `libs.event_base_new()` what is
actually called is `::event_base_new()`.

Many MeasurementKit objects take an optional constructor argument
that is a pointer to a `Libs` object. This allows the programmer to
simulate API failure in regress tests as shown above.

# HISTORY

The `Libs` class appeared in MeasurementKit 0.1.
