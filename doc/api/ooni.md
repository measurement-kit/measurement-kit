# NAME
ooni &mdash; OONI tests

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>
```

# DESCRIPTION

The `ooni` module contains the following submodules:

- [collector_client](ooni/collector_client.md): routines to interact with OONI's collector client
- [dns_injection](ooni/dns_injection.md): OONI's DNS injection test
- [error](ooni/error.md): OONI specific errors
- [http_invalid_request_line](ooni/http_invalid_request_line.md): OONI's HTTP invalid request line test
- [ooni_test](ooni/ooni_test.md): Base class common to all OONI tests
- [tcp_connect](ooni/tcp_connect.md): OONI's TCP connect test
- [templates](ooni/templates.md): Routines that facilitate writing OONI tests

# HISTORY

The `ooni` module appeared in MeasurementKit 0.1.0.
