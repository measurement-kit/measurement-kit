# NAME
DnsInjection &mdash; OONI dns-injection test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

class DnsInjection : public OoniTest {};

} // namespace ooni
} // namespace mk
```

# DESCRIPTION

The `DnsInjection` class allows to run OONI dns-injection test. In addition
to `OoniTest`, this class honours the following options:

- *"backend"* (string): address and port (separated by colon) of the server where
  a DNS server is *not* actually running such that, if we receive a response, such
  response has necessarily been injected.

Also, this test requires an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::DnsInjection()
    .set_options("backend", "127.0.0.1:53")
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.json")
    .increase_verbosity()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::DnsInjection()
    .set_options("backend", "127.0.0.1:53")
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

The `DnsInjection` class appeared in MeasurementKit 0.1.0.
