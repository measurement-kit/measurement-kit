# NAME
net -- Low-level networking library

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/net.hpp>
```

# STABILITY

2 - Stable

# DESCRIPTION

The `net` libraries contains low level networking functionality.

This library contains the following modules:

- [buffer](net/buffer.md): Buffer for incoming/outgoing data
- [connect](net/connect.md): Functions to establish connections
- [error](net/error.md): Network-level errors
- [transport](net/transport.md): Abstract interface for connected sockets

# HISTORY

The `net` library appeared in MeasurementKit 0.1.0.
