# NAME
Bufferevent -- RAII wrapper for libevent bufferevent

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

// Create empty bufferevent
Bufferevent bufev;

// Create bufferevent attached to socket
//   event_base *evbase
//   evutil_socket_t sockfd
Bufferevent bufev(evbase, sockfd, BEV_OPT_CLOSE_ON_FREE);

// Attach bufferevent to another socket
bufev.make(evbase, sockfd2, BEV_OPT_CLOSE_ON_FREE);

// Convert if allocated otherwise throws
bufferevent *p = bufev;
```

# DESCRIPTION

The `Bufferevent` object is a smart pointer possibly containing a
libevent `bufferevent` structure. It manages the life cycle of the
underlying structure. It throws if the `Bufferevent` class is bound
to a `nullptr` structure (this happens if you use the default
constructor and never call `make()`). Of course the pointed structure,
if allocated, is freed when the object goes out of scope.

# HISTORY

The `Bufferevent` class appeared in MeasurementKit 0.1.
