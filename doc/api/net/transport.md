# NAME
Transport &mdash; TCP-like connection

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class Transport {
  public:
    virtual void emit_connect() = 0;
    virtual void emit_data(Buffer buf) = 0;
    virtual void emit_flush() = 0;
    virtual void emit_error(Error err) = 0;

    virtual void on_connect(std::function<void()>) = 0;
    virtual void on_data(std::function<void(Buffer)>) = 0;
    virtual void on_flush(std::function<void()>) = 0;
    virtual void on_error(std::function<void(Error)>) = 0;

    virtual void record_received_data() = 0;
    virtual void dont_record_received_data() = 0;
    virtual Buffer &received_data() = 0;
    virtual void record_sent_data() = 0;
    virtual void dont_record_sent_data() = 0;
    virtual Buffer &sent_data() = 0;

    virtual void set_timeout(double) = 0;
    virtual void clear_timeout() = 0;

    virtual void write(const void *, size_t) = 0;
    virtual void write(std::string) = 0;
    virtual void write(Buffer) = 0;

    virtual void close(std::function<void()>) = 0;

    virtual std::string socks5_address() = 0;
    virtual std::string socks5_port() = 0;
};

} // namespace net
} // namespace mk

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb);

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb,
           Var<Reactor> reactor = Reactor::global());

void read(Var<Transport> t, Var<Buffer> buff, Callback<Error> callback,
          Var<Reactor> reactor = Reactor::global());

```

# DESCRIPTION

The `Transport` is a TCP like connection to another endpoint. You typically
construct a transport using the `net::connect()` factory method.

A `Transport` is an event emitter class, meaning that it defines functions to
emit and intercept specific events. The following events are defined:

- *connect*: emitted when the connection is established (this is currently only
  used when you connect a SOCKS5 transport)
- *data*: emitted when data was received from the network
- *flush*: emitted when the output buffer is empty
- *error*: emitted when a error occurs

By default the handlers of all these events are empty (i.e. they do nothing
when they are called).

In addition the `Transport` also allows to record the received and the sent data,
and to set and clear the I/O timeout.

The `write()` family of methods allow to write respectively a C style buffer, a
C++ string, and a `Buffer` object. All these functions fill the send buffer, and
a corresponding *flush* event would be emitted when it is empty.

The `close()` method initiates a close operation. The callback passed as first
argument would be called when the `Transport` have been actually closed. We
recommend always making sure that a `Transport` has been closed before continuing,
as shown in the following snippet:

```C++
/* Some code */
transport->close([=]() {
    /* Continuation code after transport was closed */
});
```

Failure to do so MAY result in memory errors, since it is an underlying assumption
of MeasurementKit code that the `Var<Transport>` object would only exit from the
scope after the `Transport` itself has actually been closed.

The `socks5_address()` and `socks5_port()` methods return respectively the SOCKS5
address and port being used, if any. Otherwise the empty string is returned.

The `write()` function is syntactic sugar that schedules an asynchronous write
operation on a transport and calls the provided callback when either an error
occurs or the underlying buffer has been flushed.

The `readn()` function is syntactic sugar that starts an asynchronous read
operation on a transport and only returns whether either `n` bytes have been
read or an error occurred. The `read()` function is a wrapper that calls
the `readn()` function with `n` equal to `1`.

# HISTORY

The `Transport` class appeared in MeasurementKit 0.1.0. The `read()`, `readn()`,
and `write()` functions were added in v0.2.0.
