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
    virtual ~Transport();

    /*
     * As emitter:
     */

    virtual void emit_connect() = 0;
    virtual void emit_data(Buffer buf) = 0;
    virtual void emit_flush() = 0;
    virtual void emit_error(Error err) = 0;

    virtual void on_connect(Callback<>) = 0;
    virtual void on_data(Callback<Buffer>) = 0;
    virtual void on_flush(Callback<>) = 0;
    virtual void on_error(Callback<Error>) = 0;

    virtual void close(Callback<>) = 0;

    /*
     * As recorder:
     */

    virtual void record_received_data() = 0;
    virtual void dont_record_received_data() = 0;
    virtual Buffer &received_data() = 0;

    virtual void record_sent_data() = 0;
    virtual void dont_record_sent_data() = 0;
    virtual Buffer &sent_data() = 0;

    /*
     * As writable:
     */

    virtual void write(const void *, size_t) = 0;
    virtual void write(std::string) = 0;
    virtual void write(Buffer) = 0;

    /*
     * As SOCKS5:
     */

    virtual std::string socks5_address() = 0;
    virtual std::string socks5_port() = 0;

    /*
     * As pollable:
     */

    virtual void set_timeout(double timeo) = 0;
    virtual void clear_timeout() = 0;

    /*
     * As connectable:
     */


    virtual double connect_time() = 0;
    virtual void set_connect_time_(double) = 0;
    virtual std::vector<Error> connect_errors() = 0;
    virtual void set_connect_errors_(std::vector<Error>) = 0;
    virtual dns::ResolveHostnameResult dns_result() = 0;
    virtual void set_dns_result_(dns::ResolveHostnameResult) = 0;

  protected:
    virtual void adjust_timeout(double timeo) = 0;

    virtual void shutdown() = 0;

    virtual void start_reading() = 0;
    virtual void stop_reading() = 0;

    virtual void start_writing() = 0;
};

} // namespace net
} // namespace mk

/*
 * Syntactic sugar:
 */

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb);

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb,
           Var<Reactor> reactor = Reactor::global());

void read(Var<Transport> t, Var<Buffer> buff, Callback<Error> callback,
          Var<Reactor> reactor = Reactor::global());

```

# DESCRIPTION

The `Transport` is a TCP like connection to another endpoint. You typically
construct a transport using the `net::connect()` factory method.

## As event emitter

A `Transport` is an event emitter class, meaning that it defines functions to
emit and intercept specific events. The following events are defined:

- *connect*: emitted when the connection is established (this is currently only
  used when you connect a SOCKS5 transport)
- *data*: emitted when data was received from the network
- *flush*: emitted when the output buffer is empty
- *error*: emitted when a error occurs

By default the handlers of all these events do nothing. You can set handlers
using the `on_EVENT()` methods. You can emit events using the `emit_EVENT()`
methods.

The `close()` method initiates a close operation. Doing that puts the transport
in a state where further emitted events are not delivered. Moreover, once
closed, the transport would not attempt to change the state of the underlying
I/O mechanism (i.e. it will not invoke any method described below as part of
the "pollable" interface of a `Transport`). Once `close()` is in progress,
further attempts to `close()`, will raise an exception. The callback passed
as the first argument will be invoked by the destructor of `Transport`. We
recommend always making sure that a `Transport` was closed correctly, as shown
by this code snippet:

```C++
/* Some code */
transport->close([=]() {
    /* Continuation code after transport has been closed */
});
```

This will guarantee that you notice if a `Transport` is not actually closed
because there are still references to `Var<Transport>` around.

## As data recorder

In addition the `Transport` also allows to record the received and the sent
data, using specific methods.

## As writable

The `write()` family of methods allow to write respectively a C style buffer, a
C++ string, and a `Buffer` object. All these functions fill the send buffer, and
a corresponding *flush* event would be emitted when it is empty.

## As SOCKS5

The `socks5_address()` and `socks5_port()` methods return respectively the SOCKS5
address and port being used, if any. Otherwise the empty string is returned.

## As pollable descriptor

This set of methods defines how the transport interfaces with the
underlying async I/O implementation. Since this is how the transport
implementation should interface with the lower level of abstraction
most methods in here are `protected`.

The `set_timeout()` and `clear_timeout()` methods respectively set the
timeout to be used for I/O operations, as a floating point number of
seconds `timeo`, and clear such timeout. The read timeout fires when no
data is received after `timeo` seconds, likewise the write timeout fires
if a pending write operation does not complete within `timeo` seconds. By
default, measurement-kit sets a 30 seconds timeout. Clearing the timeout
causes I/O operation to wait for a number of seconds possibly
equal to the lifespan of the universe, in the best (or is it worst?)
case scenario.

The `adjust_timeout()` method is called by `set_timeout()` or `clear_timeout()`
to enforce setting the timeout. This method MUST NOT be called by the generic
code after the transport has been closed. Passing a negative value implies
that the timeout is to be cleared.

The `shutdown()` method implementation depends on the underlying I/O
mechanism. It is automatically called as part of `close()`. The implementation
SHOULD ensure that this method is idempotent.

The `start_reading()` and `stop_reading()` methods respectively enable and
disable reading from the internal socket. The implementation MUST automatically
enable (disable) reading when you set (clear) the `on_data()` callback. This
method MUST NOT be called by the generic code after the transport has been
closed.

The `start_writing()` method initiates an asynchronous write operation that
is terminated by emitting the `FLUSH` event when the output buffer becomes
empty. The implementation MUST automatically start writing when you call
any `write()` method. This method MUST NOT be called by the generic code after
the transport has been closed.

## As connectable

This set of methods allow to get various connect-time values, like the
time required to connect (which approximates the minimum RTT), the errors
experienced when connecting (for example, the DNS may return more than
one address and some of them may be unreachable, but others work), and the
result of the DNS query for the provided hostname.

It also contains semi-hidden methods to set such values when connecting.

## Syntactic sugar

The `write()` function is syntactic sugar that schedules an asynchronous write
operation on a transport and calls the provided callback when either an error
occurs or the underlying buffer has been flushed.

The `readn()` function is syntactic sugar that starts an asynchronous read
operation on a transport and only returns whether either `n` bytes have been
read or an error occurred. The `read()` function is a wrapper that calls
the `readn()` function with `n` equal to `1`.

# BUGS

We typically pass around the `Transport` as a `Var<Transport>`. Since `Var` is
a shared pointer, this means that a `Transport` may not be closed, after the
`close()` method is called, unless all `Var`s pointing to it are cleared. For
this reason, we added a callback to `close()`, so you know the code is not
working correctly if the program stalls when it would be supposed to continue
after `close()`. This is a bit rough but it currently works. In the future, it
will be interesting to experiment with weak shared pointers such that we have
a single permanent shared reference in the `Reactor` and the user only has
access to a weak shared pointer that becomes a real shared pointer only when
the object is temporarily needed (and that handles the case where the original
object is expired so it's not possible to temporarily acquire it).

# HISTORY

The `Transport` class appeared in MeasurementKit 0.1.0. The `read()`, `readn()`,
and `write()` functions were added in v0.2.0.
