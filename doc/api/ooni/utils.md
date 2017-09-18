# NAME
util &mdash; Useful functions

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

void ip_lookup(
        Callback<Error, std::string> callback,
        Settings settings = {},
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global()
);

void resolver_lookup(
        Callback<Error, std::string> callback,
        Settings = {},
        SharedPtr<Reactor> reactor = Reactor::global(),
        SharedPtr<Logger> logger = Logger::global()
);

report::Entry represent_string(const std::string &s);

} // namespace ooni
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `ip_lookup` function returns the public IP address of the host where
MeasurementKit is running. It does so by querying a remote service. As of
version 0.4.0 such service is `geoip.ubuntu.com`. The `callback` passed
as first argument is invoked when either the IP address has ben resolved
or an error occurred. In case of success the first argument of such callback
will receive `NoError` and the second argument will receive the IP public
IP address of the local machine; otherwise, the first argument will be the
error that occurred and the second argument will be an empty string. The
caller can also specify as optional `ip_lookup` arguments: additional
settings, a reactor and a logger.

The `resolver_lookup` function retrieves the address of the DNS resolver
used by the host where MeasurementKit is running. It does so by querying a
remote service. As of version 0.4.0 such service is `whoami.akamai.net`. The
first argument passed to `resolver_lookup` is a callback invoked when the
DNS resolver IP address is found or an error occurred. On success, the first
argument passed to the callback will be `NoError` and the second argument
will be the IP address of the DNS resolver. On failure, the first argument
is the error that occurred and the second argument is an empty string. The
caller can also optionally pass to `resolver_lookup` extra settings, a
reactor and a logger. Note that the IP address returned by this function
MAY NOT be the IP address of the DNS name server configured by the client,
especially for large ISPs. For example, if you use `8.8.8.8` as your DNS
name server, this function will return an IP address owened by Google
but different from `8.8.8.8`.

The `represent_string` function represent the `s` passed as argument
such that it can be serialized as JSON. Is the string is ASCII or UTF-8, it
is not changed. Otherwise, this function returns a JSON object where the
`format` key maps to the `base64` string and the `data` key maps to the base64
representation of the original string.

# HISTORY

The `ooni::util` submodule was documented firstly in MeasurementKit 0.4.0.
