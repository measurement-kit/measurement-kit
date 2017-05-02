# NAME
version &mdash; MeasurementKit version

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common/version.h>

#define MEASUREMENT_KIT_VERSION "0.7.0-dev"

const char *mk_version(void);
```

# STABILITY

2 - Stable

# DESCRIPTION

This header defines the `MEASUREMENT_KIT_VERSION` macro that allows
the programmer to know MeasurementKit version number.

It also contains a *C linkage* function that can be used to programmatically
retrieve the library version number. The function has C linkage so that it
can be easily used from languages with foreign functions interface.

# HISTORY

The `version.hpp` header appeared in MeasurementKit 0.2.0.
