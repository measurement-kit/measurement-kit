# NAME
HttpInvalidRequestLine &mdash; OONI http-invalid-request-line test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

class HttpInvalidRequestLine : public OoniTest {};

} // namespace ooni
} // namespace mk
```

# DESCRIPTION

The `HttpInvalidRequestLine` class allows to run OONI
http-invalid-request-line test. In addition
to `OoniTest`, this class honours the following options:

- *"backend"* (string): URL of the server where an `echo`
  backend is running, such that we can evaluate what is
  echoed back and search for tampering.

This test does not require an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::HttpInvalidRequestLine()
    .set_options("backend", "http://127.0.0.1/")
    .set_input_filepath("test/fixtures/hosts.txt")
    .set_output_filepath("results.json")
    .increase_verbosity()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Run async test
mk::ooni::HttpInvalidRequestLine()
    .set_options("backend", "http://127.0.0.1/")
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

The `HttpInvalidRequestLine` class appeared in MeasurementKit 0.1.0.
