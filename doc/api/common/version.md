# NAME
version &mdash; MeasurementKit version

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common/version.h>

#define MK_VERSION "0.6.3"
#define MK_VERSION_FULL "v0.6.3-6-abcdef"

#define MEASUREMENT_KIT_VERSION MK_VERSION /* Backward compat. */

const char *mk_version(void);
const char *mk_version_full(void);
const char *mk_openssl_version(void);
const char *mk_libevent_version(void);
```

# STABILITY

2 - Stable

# DESCRIPTION

This header defines the `MK_VERSION` macro that allows the programmer
to know MeasurementKit version number. The `MK_VERSION_FULL` macro
contains the output of `git describe --tags` and is hence more precise
than the version contained by `MK_VERSION`. Note that the two version numbers
will be equal if the current build is a specific tag; otherwise, the version
with git tag will contain more precise information. Specifically it will
indicate the closest tag, the number of commits since that tag, and the HEAD
from which the current release has been built.

It also contains a *C linkage* function, `mk_version()` that can be used
to programmatically retrieve the library version number. The function has C
linkage so that it can be easily used from languages with foreign functions
interface. There is also `mk_version_full()` that returns the more
precise version containing the output of `git describe --tags`.

It also contains *C linkage* functions that tell you the version of OpenSSL
and libevent we are compiling measurement-kit with.

# HISTORY

The `version.hpp` header appeared in MeasurementKit 0.2.0. Support for
knowing the precise git tag and dependencies versions was added during the
v0.6 release cycle.
