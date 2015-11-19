# NAME
Evbuffer -- RAII wrapper for libevent evbuffer

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

Evbuffer evbuf;
evbuffer *p = evbuf; // Conversion done automatically
```

# DESCRIPTION

The `Evbuffer` object is a container for an `evbuffer` structure of
libevent. The advantages of the container are that allocation is lazy
and on-demand. Moreover the container checks whether allocation of
the `evbuffer` fails (and raises an exception if so). Also the container
automatically frees the underlying evbuffer when it goes out of scope.

# HISTORY

The `Evbuffer` class appeared in MeasurementKit 0.1.
