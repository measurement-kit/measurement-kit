# NAME
Reactor &mdash; Dispatcher of I/O events

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

#define MK_POLLIN /* Unspecified; used by pollfd() */
#define MK_POLLOUT /* Unspecified; used by pollfd() */

class Reactor {
  public:
    static Var<Reactor> make();
    static Var<Reactor> global();

    void call_later(double delay, Callback<> &&cb);
    void call_soon(Callback<> &&cb);

    void run_with_initial_event(Callback<> &&cb);
    void run();
    void stop();

    void pollfd(
            socket_t sockfd,
            short events,
            double timeout,
            Callback<Error, short> &&callback,
    );

    // Backward compatibility aliases
    void loop() { run(); }
    void break_loop() { stop(); }
    void loop_with_initial_event(Callback<> &&func) {
        run_with_initial_event(std::move(func));
    }
    void pollfd(socket_t sockfd, short events, Callback<Error, short> &&cb,
                double timeout = -1.0) {
        pollfd(sockfd, events, timeout, std::move(cb));
    }
}

/* Functional interface (by default using the global reactor): */

void call_later(
        double delay,
        Callback<> callback,
        Var<Reactor> reactor = Reactor::global()
);

void call_soon(
        Callback<> callback,
        Var<Reactor> reactor = Reactor::global()
);

// Syntactic sugar
void run(Var<Reactor> reactor = Reactor::global()) {
    reactor->run();
}
void stop(Var<Reactor> reactor = Reactor::global()) {
    reactor->stop();
}
void run_with_initial_event(Callback<> &&func,
                            Var<Reactor> reactor = Reactor::global()) {
    reactor->run_with_initial_event(std::move(func));
}

// Backward compatibility aliases
void loop(Var<Reactor> reactor = Reactor::global()) {
    run(reactor);
}
void break_loop(Var<Reactor> reactor = Reactor::global()) {
    stop(reactor);
}
void loop_with_initial_event(Callback<> &&func,
                             Var<Reactor> reactor = Reactor::global()) {
    run_with_initial_event(std::move(func));
}

```

# STABILITY

2 - Stable

# DESCRIPTION

The `Reactor` abstract interface dispatches I/O events. Most MeasurementKit
objects refer to a specific `Reactor` object.

The `make()` and `global()` factories return a reactor allocated
on the heap whose lifecycle is manager using a `Var<>` smart pointer.
Specifically, `make()` allocates a new reactor and `global()` returns
a reference to the global reactor.

You can schedule a callback to be called immediately using the `call_soon`
method that takes such callback as its first argument.

You can schedule a callback to be called after a specific number
of seconds using `call_later`. The first argument is the number of
seconds to wait before calling the callback. The second argument
is the callback itself.

You can ask the reactor to wait for I/O on a specific socket
descriptor using `pollfd`. In general, beware that this MAY be less
efficient than using the `Transport` API (see the `net` package)
for doing asynchronous I/O. To do so, use the `pollfd` overloaded
family of methods:

The first overload of `pollfd` is the preferred form and takes the
following arguments.  The first argument is a socket file descriptor.
The second argument is the bitmaks of events you want to monitor
for; it can be `MK_POLLIN`, to wait for the socket being readable,
`MK_POLLOUT` to wait for the socket being writable, or `MK_POLLIN
| MK_POLLOUT` to wait for both. The third argument is the timeout
after which you want to stop waiting for I/O. Pass a negative value
to indicate that you don't want any timeout checking. The fourth
argument is the callback to be called when either the socket is
ready for I/O or there has been a timeout. The first argument passed
to the callback indicates whether there was an error (typically
`TimeoutError`) or not (in such case the error will be `NoError`).
The second argument passed to the callback indicate whether the
socket is readable (`MK_POLLIN`), writable (`MK_POLLOUT`), or both
(`MK_POLLIN | MK_POLLOUT`).

In second overload of `pollfd` the callback is the third argument and
the timeout is optional and is the fourth argument. This overload is
implemented by calling the previous overload with swapped third and fourth
arguments. This overload is meant as a convenience when you don't want
to specify any timeout. It also preserves backward compatibility with
versions of MeasurementKit lower than v0.7.0.

Other available methods (typically to be called in `main()`) are:

The `run_with_initial_event` method runs the reactor and calls the specified
callback when the reactor is running. This is equivalent to calling
`call_soon()` with the target callback, followed by `run()`.

The `run` method runs the reactor. This is a blocking method that does not
return until the reactor runs out I/O events to poll for and/or pending
(possibly delayed) calls. You can also stop a running reactor explicitly
by calling `stop`. Calling `run` when the reactor is already running
throws a `std::runtime_error` exception.

The `stop` method stop the reactor. This is an idempotent method that you
can call many times. This method MAY return while the reactor is still
running. That is, it only tells the reactor to stop but it does not provide
the guarantee that, when it returns, the reactor is already stopped.

The `loop_with_initial_event`, `loop`, and `break_loop` methods are deprecated
aliases for, respectively, `run_with_initial_event`, `run`, and `stop`.

The `get_event_base` is a deprecated method that returns the underlying
`event_base` used by the reactor. It is deprecated because it exposes in
great detail our dependency on libevent. Ideally, this method should be
a method of the specific implementation of the reactor, available only
when you downcast from reactor to its specific implementation.

In addition, this module exposes also syntactic sugar functions:

The `call_soon`, `call_later`, `loop_with_initial_event`, `loop`, `loop_once`,
`break_loop`, `run_with_initial_event`, `run`, and `stop` functions are
syntactic sugar that call the respective method of the global reactor (i.e.
the one obtained with `Reactor::make()`.

# GUARANTEES

1. it is safe to call `global` and `global_remote` concurrently from
   multiple threads.

2. all reactor methods are thread safe.

3. the reactor MAY actually be a proxy for a real reactor multiplexing I/O
   running from a background thread. In such case, disposing of the foreground
   object has no effect on the callbacks scheduled in the real reactor. To
   guarantee this, the signature of functions taking callbacks is such that
   they must be moved (explicitly or implicitly), thus giving their ownership
   to the (possibly running in a background thread) reactor. As a result,
   once a callback is scheduled, there should be no shared state between
   different threads. We recommend, in such case, to move any state you
   might need into the callback closure, e.g.:

```C++
        reactor->call_soon([state = std::move(state)]() {
            // Possibly running in the background I/O thread. Has single
            // ownership of the shared state.
        });
```

# CAVEATS

1. the `pollfd` interface is typically less efficient that using the
   `Transport` based interface implemented in `net`.

2. callbacks MAY be called from another thread context.

3. there is currently no way to know whether the reactor will run the
   callbacks in the same or in another thread context.

4. calling `stop` before calling `run` has no effect and will typically lead
   to your program enter the I/O loop and suddenly leaving it because there
   is nothing to do. When you want to run "initialization" actions in the
   context of the I/O loop you should use instead the following pattern:

```C++
int main(int argc, char **argv) {
    Reactor reactor = Reactor::make();

    // Allocate objects on the stack before calling the blocking
    // `run_with_initial_event` method of `reactor`.
    Object obj{reactor};
    Foo bar;

    // Capture by reference (`&`) because this method is blocking
    reactor->run_with_initial_event([&]() {
        obj.action([&]() {
            bar.baz();
            reactor->stop();
        });
    });
}
```

# HISTORY

The `Reactor` class appeared in MeasurementKit 0.1.0, named `Poller`.
It was renamed `Reactor` in MeasurementKit 0.2.0. As of MK v0.2.0,
the `Poller` still exists as a specific implementation of the
`Reactor` interface described in this manual page. The `Reactor`
was significantly improved as part of MK v0.4.0 and MK v0.7.0.

Before MK v0.8.0, `Reactor::run()` was blocking until `stop` was
called. After, `Reactor::run()` will return as soon as there aren't
pending I/O events or (possibly delayed) calls.
