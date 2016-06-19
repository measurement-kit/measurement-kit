# NAME
ooni -- OONI tests library

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>
```

# STABILITY

2 - Stable

# DESCRIPTION

The `ooni` libraries contains OONI tests.

This library contains the following modules:

- [collector_client](ooni/collector_client.md): routines to interact with OONI's collector client
- [dns_injection](ooni/dns_injection.md): OONI's DNS injection test
- [error](ooni/error.md): OONI specific errors
- [http_invalid_request_line](ooni/http_invalid_request_line.md): OONI's HTTP invalid request line test
- [ooni_test](ooni/ooni_test.md): Base class common to all OONI tests
- [tcp_connect](ooni/tcp_connect.md): OONI's TCP connect test
- [templates](ooni/templates.md): Routines that facilitate writing OONI tests

# HISTORY

The `ooni` library appeared in MeasurementKit 0.1.0.
