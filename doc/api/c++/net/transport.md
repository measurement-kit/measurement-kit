# NAME
Transport -- TCP-like connection

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/net.hpp>

mk::Maybe<mk::Var<mk::net::Transport>> mt = net::connect({
    {"address", "www.google.com"},
    {"port", "80"}
});
if (!mt) throw mt.as_error();
mk::Var<mk::net::Transport> transport = mt.as_value();
// Use transport...

transport->on_connect([]() {
    /* connection established */
});
transport->on_data([](mk::net::Buffer buff) {
    /* data received */
});
transport->on_flush([]() {
    /* all queued data was sent */
});
transport->on_error([](mk::Error error) {
    /* handle error that occurred */
});

transport->set_timeout(7.14);
transport->clear_timeout();

mk::net::Buffer buff;
transport->send("sassaroli", 5);
transport->send(std::string("sassaroli"));
transport->send(buff);

transport->close();

std::string s = transport->socks5_address();  // empty string if no proxy
std::string s = transport->socks5_port();     // ditto

/* event emitters: */
mk::net::Buffer buffer;
transport->emit_connect();
transport->emit_data(buffer);
transport->emit_flush();
transport->emit_error(mk::net::EOFError());
```

# DESCRIPTION

The `Transport` is a TCP like connection to another endpoint. You typically
construct a transport using the `net::connect()` factory method, which accepts
the following options:

- *address*: address or domain name to connect to

- *dumb_transport*: if specified tells the factory method to create a dumb
transport not connected to any endpoint and only suitable for debugging (default:
unspecified)

- *family*: could be one of the following:

    - *PF_INET*: only resolve and use IPv4 addresses

    - *PF_INET6*: only resolve and use IPv6 addresses

    - *PF_UNSPEC*: try with IPv4 first and if it fails then try with IPv6
      (this is the default)

    - *PF_UNSPEC6*: try with IPv6 first and if it fails then try with IPv4

- *port*: port to connect to

- *socks5_proxy*: string specifying which SOCKS5 proxy to use, with
address and port separated by a colon (default: unspecified)

- *timeout*: timeout for I/O operations (default: 30 s)

# BUGS

Options can only be specified as strings. It would be nice to allow for them
to be either string or numbers, depending on their semantic.

The return value of `connect()` is ugly because it is made of two templates,
i.e. `Maybe<>` and `Var<>`. It is possible that for this and other reasons we
will consider making `Transport` movable and copyable such that `connect()`
will return just the more readable and manageable `Maybe<Transport`>.

# HISTORY

The `Transport` class appeared in MeasurementKit 0.1.0.
