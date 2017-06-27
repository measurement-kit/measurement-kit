# NAME
platform &mdash; Get current platform

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common/platform.h>

const char *mk_platform(void);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `mk_platform` C-linkage function returns the name of the current
platform. The name will be one of:

- android
- ios
- linux
- macos
- unknown
- windows

# HISTORY

The `platform.h` header appeared in MeasurementKit 0.7.0.
