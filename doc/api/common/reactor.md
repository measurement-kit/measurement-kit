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
    static Var<RunningReactor> make_remote();
    static Var<RunningReactor> global_remote();

    void call_later(double delay, Callback<> cb);
    void call_soon(Callback<> cb);

    void loop();
    void break_loop();
    void loop_with_initial_event(Callback<> func);

    void run_with_initial_event(Callback<> cb);
    void run();
    void stop();

    void pollfd(
            socket_t sockfd,
            short events,
            double timeout,
            Callback<Error, short> callback,
    );
    void pollfd(
            socket_t sockfd,
            short events,
            Callback<Error, short> callback,
            double timeout = -1.0
    );
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

void run(Var<Reactor> reactor = Reactor::global());
void stop(Var<Reactor> reactor = Reactor::global());
void run_with_initial_event(
        Callback<> func,
        Var<Reactor> reactor = Reactor::global()
);

// Deprecated since v0.4.x
void loop(Var<Reactor> reactor = Reactor::global());
void break_loop(Var<Reactor> reactor = Reactor::global());
void loop_with_initial_event(
        Callback<> func,
        Var<Reactor> reactor = Reactor::global()
);

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

The reactor returned by `make()` and `global()` is meant to be used
in the local thread (hereinafter called *regular* reactor). It is
also possible to obtain a reactor running in a background thread
using `make_remote()` and `global_remote()`.  This type of reactor
is hereinafter called *global* reactor. Beware that a global reactor
MAY NOT implement all reactor methods (more info on that below).

The following methods are implemented both by regular and remote reactors:

You can schedule a callback to be called immediately using the `call_soon`
method that takes such callback as its first argument.

You can schedule a callback to be called after a specific number of
seconds. The first argument is the number of seconds to wait before calling
the callback. The second argument is the callback itself.

You can ask the reactor to wait for I/O on a specific socket descriptor. In
general, beware that this MAY be less efficient than using the
`Transport` API (see the `net` package) for doing asynchronous I/O. To do
so, use the `pollfd` overloaded family of methods.

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

The following methods are only implemented by regular reactors and, if
you call them on a remote reactor, they throw `std::logic_error`:

The `run_with_initial_event` method runs the reactor and calls the specified
callback when the reactor is running. This is a blocking method that does
not return until you call the `stop` method.

The `run` method runs the reactor. This is a blocking method that does not
return until you call the `stop` method.

The `stop` method stop the reactor. This is an idempotent method that you
can call many times. This method MAY return while the reactor is still
running. That is, it only tells the reactor to stop but it does not provide
the guarantee that, when it returns, the reactor is already stopped.

The `loop_with_initial_event`, `loop`, and `break_loop` methods are deprecated
aliases for, respectively, `run_with_initial_event`, `run`, and `stop`.

The `get_event_base` is a deprecated method that returns the underlying
`event_base` used by the reactor.

The `loop_once` method is a deprecated method that runs just one iteration
of the I/O loop and then returns immediately.

In addition, this module exposes also syntactic sugar functions:

The `call_soon`, `call_later`, `loop_with_initial_event`, `loop`, `loop_once`,
`break_loop`, `run_with_initial_event`, `run`, and `stop` functions are
syntactic sugar that call the respective method of the global reactor (i.e.
the one obtained with `Reactor::make()`.

# GUARANTEES

1. it is safe to call `global` and `global_remote` concurrently from
   multiple threads

2. all reactor methods are thread safe

3. if the reactor is obtained through `remote` (or through `global_remote`),
   you can safely dispose of the returned object after you have called all
   the methods you needed, and be confident that the background I/O loop will
   continue to run as you instructed without hazards. Of course, disposing
   of such remote reactor also means that you cannot stop it, but there may
   be use cases where you want that (typically on mobile apps where you would
   like to have a background thread dedicated to I/O).

# CAVEATS

1. the `pollfd` interface is typically less efficient that using the
   `Transport` based interface implemented in `net`

2. if you are using a remote reactor, callbacks will be called in the
   thread context of the background thread dedicated to I/O

3. if you are using a remote reactor, make sure you *move* callbacks
   passed to its methods, otherwise the destructor of the callbacks
   will be called from multiple thread contexts (and you know what this
   means, don't you?). The following C++14 pattern is safe and SHOULD
   be used throughout the code for increased robustness:

```C++
    auto obj = Var<Object>::make();
    auto reactor = Reactor::global_remote();
    // Move the object to enforce single ownership
    reactor->call_later(3.14, [copy = std::move(obj)]() {
        // Beware: this method will be called from a background thread.
        copy->something();
    });
```

   Whenever you *know* that the object will only be used from within
   the context of the I/O thread (be it background or not), you can
   also use the following C++11 compatible pattern:

```C++
    auto obj = Var<Object>::make();
    auto reactor = Reactor::global();
    reactor->call_later(3.14, [obj]() { obj->something(); });
```

4. calling `stop` before calling `run` has no effect and will typically lead
   to your program hanging for an infinite amount of time. When you want to
   run "initialization" actions in the context of the I/O loop you should use
   instead the following pattern:

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

# BUGS

It would probably be optimal to refactor MK such that we have two different
interfaces for the "base" reactor (on which calling both regular and remote
reactor methods is safe) and for the "regular" reactor. Code in tests etc.
should then use the "base" interface and the "regular" interface should only
be used in few places where it makes sense to start the I/O loop etc.

# HISTORY

The `Poller` class appeared in MeasurementKit 0.1.0. It was renamed
`Reactor` in MeasurementKit 0.2.0. As of MK v0.2.0, the `Poller`
still exists as a specific implementation of the `Reactor` interface
described in this manual page. The `Reactor` was significantly
improved as part of MK v0.4.0 and MK v0.7.0.
