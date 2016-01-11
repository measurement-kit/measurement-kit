# NAME
Error -- A traceroute error.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/traceroute.hpp>


mk::traceroute::SocketCreateError;         // socket creation error
mk::traceroute::SetsockoptError;           // Setsockopt error
mk::traceroute::ProbeAlreadyPendingError;  // a probe is already pending
mk::traceroute::PayloadTooLongError;       // payload is too long
mk::traceroute::StorageInitError;          // error in storare initialization
mk::traceroute::BindError;                 // bind error
mk::traceroute::EventNewError;             // event new error
mk::traceroute::SendtoError;               // sendto error
mk::traceroute::NoProbePendingError;       // no probe is pending
mk::traceroute::ClockGettimeError;         // ClockGettime error
mk::traceroute::EventAddError;             // eventadd error
mk::traceroute::SocketAlreadyClosedError;  // socket is already closed
```

# DESCRIPTION

Errors that occurr in the `traceroute` library of MeasurementKit.

# HISTORY

Traceroute specific errors appeared in MeasurementKit 0.1.0.
