# NAME
ProberInterface - Interface of the traceroute module.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/traceroute.hpp>

// Constructor

auto prober = mk::traceroute::Prober<ProberInterface>(true, 11829);

prober.send_probe("8.8.8.8", 33434, ttl, payload, 1.0); // send a probe

prober.on_result([&prober, &ttl, &payload](ProbeResult r) {
    /* Do something when send_probe gives a result */
});

prober.on_timeout([&prober, &ttl, &payload]() {
    /* do something when send_probe has a timeout */
});

prober.on_error([&prober, &ttl, &payload](Error err) {
    /* handle error that occurred */
});
```

# DESCRIPTION

This is the interface of the traceroute module. 

This module is disabled on non Linux platforms.

# HISTORY

The `ProberInterface` class appeared in MeasurementKit 0.1.0.
