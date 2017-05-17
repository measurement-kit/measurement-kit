# NAME
HttpHeaderFieldManipulation &mdash; OONI http-header-field-manipulation test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

class HttpHeaderFieldManipulation : public BaseTest {};

} // namespace nettests
} // namespace mk
```

# DESCRIPTION

The `HttpHeaderFieldManipulation` class allows one to run the OONI
http-header-field-manipulation test. In addition to those from `BaseTest`,
this class honours the following options:

- *"backend"* (string): the endpoint of the backend helper which is expected
  to echo back the HTTP request as a JSON object.

This test does not require an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::HttpHeaderFieldManipulation()
    .increase_verbosity()
    .run();
```
