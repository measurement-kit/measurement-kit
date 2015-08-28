# NAME
CheckConnectivity -- Check whether there is connectivity

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

void do_something() {
    if (CheckConnectivity::is_down()) {
        fprintf(stderr, "The network is down\n");
        return;
    }
    // do something ...
}
```

# DESCRIPTION

This class is implemented directly on top of evdns, to avoid depending
from other modules, because it is often used in unit tests. In fact,
we have tests that we want to skip when the network is down.

To check whether the network is down, do:

    if (CheckConnectivity::is_down()) {
        return;
    }

This class reports that the network is up if the system-wide DNS
resolver works and returns a valid IPv4 address for ebay.com. To
reach the DNS resolver we use evdns, which we assume to be working.
If evdns is broken, the DNS resolver is not reachable or ebay.com
is no longer available, this class reports that the network is down
even if it is not actually down. All these three conditions are
quite unlikely, IMO, so this code should be robust enough.

# BUGS

The check on whether the network is down is performed only once
when `is_down()` is called for the first time. Therefore, this class
is not suitable to check whether the network is down in long running
programs.

Because it is designed for unit tests, this class uses its own event
loop. This means that, the first time that `is_down()` is called,
the current thread is blocked until the DNS response is received
or until the evdns timeout expires.

# HISTORY

The `CheckConnectivity` class appeared in MeasurementKit 0.1.
