# NAME
android &mdash; Android implementation of traceroute interface.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/traceroute.hpp>

namespace mk {
namespace traceroute {

class AndroidProber : public Prober {};

} // namespace traceroute
} // namespace mk
```

# STABILITY

1 - Experimental

# DESCRIPTION

This is the Android implementation of traceroute interface. This implementation is meant to run
on Android but can run on all Linux systems.

# EXAMPLE

```C++
auto prober = mk::traceroute::Prober<AndroidProber>(true, 11829);

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

# HISTORY

The `android` submodule appeared in MeasurementKit 0.1.0.
