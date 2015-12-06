# NAME
Error -- DNS library specific errors

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/dns.hpp>

mk::dns::FormatError;               // invalid response format
mk::dns::ServerFailedError;         // server failure
mk::dns::NotExistError;             // the name does not exist
mk::dns::NotImplementedError;       // query not implemented
mk::dns::RefusedError;              // the server refuses to reply
mk::dns::TruncatedError;            // response truncated
mk::dns::UknownError;               // internal evnds error
mk::dns::TimeoutError;              // query timed out
mk::dns::ShutdownError;             // evdns library was shut down
mk::dns::CancelError;               // user cancelled query
mk::dns::NoDataError;               // no data in the response

// Map evdns error code to a DNS error
mk::Error mk::dns::dns_error(int err_code);
```

# DESCRIPTION

These are the errors triggered by MeasurementKit DNS library.

# HISTORY

DNS error classes appeared in MeasurementKit 0.1.0.
