# NAME
Error -- Represents an error.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

mk::Error err = mk::GenericError(); // Assign to error class
mk::Error err = 17;                 // Assign error code
int code = (int) err;               // Convert error to int
bool x = (error != 17);             // Implicit conversion here
bool y = (error == 17);             // Also here
```

# DESCRIPTION

In general MeasurementKit code reports errors using the `Error` class
and uses exceptions only to report unrecoverable errors.

# HISTORY

The `Error` class appeared in MeasurementKit 0.1.0.
