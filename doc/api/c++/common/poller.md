# NAME
Poller -- Dispatcher of I/O events

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::Poller poller;
event_base *p = poller.get_event_base();
evdns_base *dp = poller.get_evdns_base();

poller.loop();                    // Blocking method to run the event loop
poller.break_loop();              // Break out of event loop
poller.loop_once();               // Just one iteration of event loop

mk::Poller *root = mk::Poller::global();
```

# DESCRIPTION

The `Poller` object dispatches I/O events. Most MeasurementKit objects
refer to a `Poller` object (be it a specific poller or the global poller).

Generally, you setup some objects making sure they refer to a poller and
then you invoke that poller's `loop()` method. Until you're satisfied and
you exit from the event loop using the `break_loop()` method.

This is the standard way of doing things in single-threaded programs. See
the documentation of `Async` to see how to run measurements in a background
thread (which is what you typically want for apps).

# HISTORY

The `Poller` class appeared in MeasurementKit 0.1.
