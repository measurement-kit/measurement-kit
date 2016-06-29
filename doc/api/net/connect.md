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
  the local system when compiling. For mobile devices, the default is the
  empty string; i.e. you must provide a sensible value yourself.

- *"net/dumb_transport"*: if this key is present a dumb transport is created (i.e. a
  transport that is not connected to any socket).

- *"net/socks5_proxy"* (string): address and port (separated by colon) of the SOCKS5
  proxy to be used for establishing the requested connection (this feature is
  still experimental as of v0.2.0).

- *"net/ssl"* (bool): whether to establish a SSL connection (default: false).

- *"net/timeout"* (double): timeout for connect and I/O operations (default: `5.0` for
  connecting and `30.0` for the established connection).

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
is the underlying libevent's `bufferevent` wrapped by `Transport`.

The `ResolveHostnameResult` class is like:

```C++
struct ResolveHostnameResult {
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;
    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;
    std::vector<std::string> addresses;
};
```

where `inet_pton_ipv4` is `true` if `address` is an IPv4 address and similarly
`inet_pton_ipv6` is `true` if `address` is an IPv6 address; `ipv4_err` and `ipv4_reply`
are the values returned by resolving `address` as a FQDN into a list of addresses;
`ipv6_err` and `ipv6_reply` have the same semantic of their IPv4 counterparts; `addresses`
is the list of the addresses that `connect()` will try to connect to. This list will
only contain a IPv4 (or IPv6) address if `address` is an IPv4 (or IPv6) address and it
will contain IPv4 addresses before IPv6 addresses (if any) when `address` instead is
a FQDN (fully qualified domain name).

The `connect_many()` function is similar to `connect()`. The main different is
that `num` parallel connections are established and passed to the callback on success. Of
course, this function would return `NoError()` only if all the parallel connect attempts
were successful, and it would close all the open connections if only some connect attempts
were successful.

# BUGS

Most MeasurementKit functions receive the `reactor` argument before the `logger`
argument, but `connect()` uses the opposite convention.

# HISTORY

The `connect` submodule appeared in MeasurementKit 0.2.0.
