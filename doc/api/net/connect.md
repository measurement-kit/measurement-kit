# NAME
connect &mdash; Routines to connect to remote hosts

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/net.hpp>

void mk::net::connect(std::string address, int port,
        Callback<Error, Var<Transport>> callback,
        Settings settings = {},
        Var<Logger> logger = Logger::global(),
        Var<Reactor> reactor = Reactor::global());

using ConnectManyCb = Callback<Error, std::vector<Var<Transport>>>;

void mk::net::connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        Var<Logger> logger = Logger::global(),
        Var<Reactor> reactor = Reactor::global());
```

# STABILITY

2 - Stable

# DESCRIPTION

The `connect()` function creates a connection to the remote `address` (which
typically is a FQDN) and `port` and calls `callback` when done. On failure,
an error is passed to the callback as its first argument; on success, the first
callback argument is `NoError` and the second argument is a `Transport` instance
wrapped by a `Var<>` smart pointer. Optionally you can also specify `settings`,
a specific `logger` and a specific `reactor`.

If `address` is a FQDN, this implementation of `connect()` will try all the
addresses obtained by resolving `address` before declaring the connect attempt
failed. In doing that, `connect()` would give preference to IPv4 addresses over
IPv6 addresses. Conversely, if `address` is already an IPv4 or IPv6 address, this
function would not attempt to resolve it and would try to connect it directly.

The behavior of `connect()` and of `Transport` s created using `connect()` can be
modified using the following `settings`:

- *"net/ca_bundle_path"* (string): path of the CA bundle to be used to verify
  SSL certificates. The default value is selected by `./configure` inspecting
  the local system when compiling. If the path is the empty string and we
  link against libressl (as is typically the case for mobile devices) we load
  libressl default CA from memory (i.e. you don't need anymore to include a
  CA file when setting up an application using measurement-kit).

- *"net/ssl_allow_dirty_shutdown"* (bool): if true, this setting treats EOF
  received on the socket without receiving a clean SSL shutdown message as a
  normal EOF. If false, this situation is reported as `SslDirtyShutdownError`.
  By default, this flag is false.

- *"net/dumb_transport"*: if this key is present a dumb transport is created (i.e. a
  transport that is not connected to any socket).

- *"net/socks5_proxy"* (string): address and port (separated by colon) of the SOCKS5
  proxy to be used for establishing the requested connection (this feature is
  still experimental as of v0.2.0).

- *"net/ssl"* (bool): whether to establish a SSL connection (default: false).

- *"net/timeout"* (double): timeout for connect and I/O operations (default: `5.0` seconds).

To provide insights into what went wrong and what went right during the connect
attempt, the `Error` returned by `connect()` contains as `context` an instance of the
following class

```C++
struct ConnectResult : public ErrorContext {
    ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    bufferevent *connected_bev = nullptr;
};
```

where `resolve_result` contains the result of resolving the `address` argument into
an IP address, `connect_result` is a vector containing an error for each failed connect
attempt (where the size of the vector would be greater than one when a domain name
mapped to multiple addresses and connecting to more than one address failed), `connected_bev`
is the underlying libevent's `bufferevent` wrapped by `Transport`. More details about the
`ResolveHostnameResult` class can be found in doc/api/dns.hpp.

The `connect_many()` function is similar to `connect()`. The main different is
that `num` parallel connections are established and passed to the callback on success. Of
course, this function would return `NoError()` only if all the parallel connect attempts
were successful, and it would close all the open connections if only some connect attempts
were successful.

# BUGS

As of MeasurementKit v0.4, `connect()` does not implement SSLv2 and SSLv3,
therefore secure connection will fail with sites that use such deprecated
versions of the protocol.

# HISTORY

The `connect` submodule appeared in MeasurementKit 0.2.0.
