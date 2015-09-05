# NAME
Error -- Represents an error.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

Error err = GenericError(); // Assign to error class
Error err = 17;             // Assign error code
int code = (int) err;       // Convert error to int
bool x = (error != 17);     // Implicit conversion here
bool y = (error == 17);     // Also here
```

# DESCRIPTION

In general MeasurementKit code reports errors using the `Error` class
and uses exceptions only to report unrecoverable errors.

# HISTORY

The `Error` class appeared in MeasurementKit 0.1.
