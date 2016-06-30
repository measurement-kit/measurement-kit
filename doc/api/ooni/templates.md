# NAME
templates &mdash; OONI test templates

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/ooni.hpp>

void mk::ooni::dns_query(Var<Entry> entry, dns::QueryType, dns::QueryClass,
               std::string query_name, std::string nameserver,
               Callback<Error, mk::dns::Message>, Settings = {},
               Var<Reactor> = Reactor::global(),
               Var<Logger> = Logger::global());

void mk::ooni::http_request(Var<Entry> entry, Settings settings, http::Headers headers,
                  std::string body, Callback<Error, Var<http::Response>> cb,
                  Var<Reactor> reactor = Reactor::global(),
                  Var<Logger> logger = Logger::global());

void mk::ooni::tcp_connect(Settings settings, Callback<Error, Var<net::Transport>> cb,
                 Var<Reactor> reactor = Reactor::global(),
                 Var<Logger> logger = Logger::global());


```

# STABILITY

1 - Experimental

# DESCRIPTION

This submodule contains OONI test templates.

The `dns_query()` function takes in input an `entry`, a query type, a query class, a
name to query for, the address and port of a nameserver (separated by colon), a `callback`
returning an error &mdash; or `NoError()` &mdash; as first argument and a DNS message
as second argument, and optional `settings`, `reactor`, and `logger`. This function will
run the specified DNS query and fill the result `entry` according to OONI conventions.

The `http_request()` function takes in input an `entry`, `settings` to be passed to
`http::request()`, HTTP `headers` and `body`, a `callback`
returning an error &mdash; or `NoError()` &mdash; as first argument and a HTTP response
as second argument, and optional `settings`, `reactor`, and `logger`. This function
will run the specified HTTP request and fill the result `entry` according to OONI
conventions.

The `tcp_connect()` function takes in input `settings` to be passed to
`net::connect()`, a `callback`
returning an error &mdash; or `NoError()` &mdash; as first argument and a `Transport`
as second argument, and optional `settings`, `reactor`, and `logger`. This function
will attempt to connect() to the endpoint specified through `settings` and fill
the result `entry` according to OONI conventions. Specifically this function honors
the following settings:

- *"host"* (string): host to connect to
- *"port"* (int) port to connect to

# HISTORY

The `templates` submodule appeared in MeasurementKit 0.2.0.
