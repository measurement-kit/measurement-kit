# NAME
Error &mdash; A traceroute error.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/traceroute.hpp>
```

# STABILITY

1 - Experimental

# DESCRIPTION

The following traceroute errors are defined:

- *mk::traceroute::SocketCreateError*: socket creation error
- *mk::traceroute::SetsockoptError*: Setsockopt error
- *mk::traceroute::ProbeAlreadyPendingError*: a probe is already pending
- *mk::traceroute::PayloadTooLongError*: payload is too long
- *mk::traceroute::StorageInitError*: error in storare initialization
- *mk::traceroute::BindError*: bind error
- *mk::traceroute::EventNewError*: event_new() error
- *mk::traceroute::SendtoError*: sendto() error
- *mk::traceroute::NoProbePendingError*: no probe is pending
- *mk::traceroute::ClockGettimeError*: clock_gettime() error
- *mk::traceroute::EventAddError*: event_add() error
- *mk::traceroute::SocketAlreadyClosedError*: socket is already closed

# HISTORY

Traceroute specific errors appeared in MeasurementKit 0.1.0.
