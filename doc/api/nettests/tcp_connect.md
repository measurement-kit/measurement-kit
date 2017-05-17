# NAME
TcpConnect &mdash; OONI tcp-connect test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

class TcpConnect : public OoniTest {};

} // namespace nettests
} // namespace mk
```

# DESCRIPTION

The `TcpConnect` class allows to run OONI tcp-connect test.

This test requires an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::TcpConnect()
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.njson")
    .increase_verbosity()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::TcpConnect()
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.njson")
    .increase_verbosity()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run([]() {
        // If needed, acquire the proper locks
        // Handle test completion
    });

```

# CAVEATS

Callbacks MAY be called from a background thread. It is your responsibility
to acquire the proper locks before manipulating shared objects.

# HISTORY

The `TcpConnect` class appeared in MeasurementKit 0.1.0. It was moved from
the `ooni` to the `nettests` namespace in v0.4.0.
