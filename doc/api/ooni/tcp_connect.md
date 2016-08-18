# NAME
TcpConnect &mdash; OONI tcp-connect test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

class TcpConnect : public OoniTest {};

} // namespace ooni
} // namespace mk
```

# DESCRIPTION

The `TcpConnect` class allows to run OONI tcp-connect test. In addition
to `OoniTest`, this class honours the following options:

- *"port"* (int): port to connect to.

Also, this test requires an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::TcpConnect()
    .set_options("port", 80)
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.json")
    .increase_verbosity()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::TcpConnect()
    .set_options("port", 80)
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.json")
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

The `TcpConnect` class appeared in MeasurementKit 0.1.0.
