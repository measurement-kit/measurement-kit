# NAME
MeekFrontedRequestsTest &mdash; OONI meek-fronted-requests test

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

class MeekFrontedRequestsTest : public BaseTest {};

} // namespace nettests
} // namespace mk
```

# DESCRIPTION

The `MeekFrontedRequestsTest` class allows one to run the OONI
meek-fronted-requests test. In addition to those from `BaseTest`,
this class honours the following options:

- *"expected_body"* (string): the HTTP payload we are expecting from the inner
  Meek-fronted server. (The libmeasurementkit default is at
  `mk::ooni::constants::MEEK_SERVER_RESPONSE`.)

This test *does* require an input file.

# EXAMPLE

```C++
// Run sync test
mk::ooni::MeekFrontedRequestsTest()
    .set_input_filepath("test/fixtures/meek_fronted_requests.txt")
    .increase_verbosity()
    .run();

```
