# NAME
Reactor &mdash; Dispatcher of I/O events

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class Reactor {
  public:
    static Var<Reactor> make();
    static Var<Reactor> global();

    virtual void call_later(double delay, Callback<> callback) = 0;
    virtual void call_soon(Callback<> callback) = 0;

    virtual void loop() = 0;
    virtual void break_loop() = 0;
    virtual void loop_with_initial_event(Callback<> func) = 0;
}

/* Functions using the global reactor: */

void call_later(double delay, Callback<> callback);
void call_soon(Callback<> callback);

void loop();
void break_loop();
void loop_with_initial_event(Callback<> func);

/* For regress tests: */

void loop_with_initial_event_and_connectivity(Callback<> func);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `Reactor` abstract interface dispatches I/O events. Most MeasurementKit
objects refer to a specific `Reactor` object.

The `make()` and `global()` factories return a reactor allocated on the heap whose
lifecycle is manager using a `Var<>` smart pointer. Specifically, `make()` allocates
a new reactor and `global()` returns a reference to the global reactor.

The `call_later()` method schedules the callback `callback` to be executed
after `delay` seconds. Use the closure of the lambda to pass objects to the
callback (and make sure that the lifecycle is managed correctly):

```C++
    Var<Object> obj(new Object);
    reactor->call_later(1.22, [=]() {
        // Here we have used `=` to pass all objects accessible on the
        // stack by value to the lambda. This pattern should be safe as
        // long as either copying the objects makes sense or you pass
        // around objects managed through a Var<> smart pointer.
        obj->something();
    });
```

The `call_soon()` method is just syntactic sugar for `call_later()` with
`0.0` passed as the first argument.

The blocking `loop()` method would run the event loop until the `break_loop()`
method is called. Calling `break_loop()` before `loop()` has no effect, in
the sense that the event loop would not be interrupted when entered. In fact,
this is a typical error (at least, it has been for me) leading to a program
that does not terminate. To avoid this, use `loop_with_initial_event()`:

```C++
int main(int argc, char **argv) {
    Reactor reactor = Reactor::make();

    // Allocate objects on the stack before calling the blocking
    // `loop_with_initial_event` method of `reactor`.
    Object ob{reactor};
    Foo bar;

    // Capture by reference (`&`) because this method is blocking
    reactor->loop_with_initial_event([&]() {
        obj.action([&]() {
            bar.baz();
            reactor->break_loop();
        });
    });
}
```

This method basically starts the event loop and ensures that the callback
provided as its only argument is invoked just after the loop is started. This
way, there is the guarantee that, even if `break_loop()` is called
immediately because `action()` completes immediately, `break_loop()`
always occurs *after* starting the loop and hence the loop is always interrupted.

In `common/reactor.hpp` we also define syntactic sugar functions that operate
on the global poller. That is `mk::loop()` is syntactic sugar for:

```C++
mk::Poller::global()->loop();
```

The `loop_with_initial_event_and_connectivity()` is a function that only starts the
event loop and calls the initial function when there is connectivity. This is used
typically in tests to avoid running tests requiring the network when the network is
not available. For example,

```C++
TEST_CASE("We can GET a specific URL") {
    loop_with_initial_event_and_connectivity([]() {
        /* We enter here only if we have working connectivity. Without this check for
           connectivity, the test would fail, not because of an error in the code rather
           because connectivity is missing. */
        http::get("http://www.example.com", [](Error e, Var<http::Response>) {
            REQUIRE(!e);
            break_loop();
        });
    });
}
```

# HISTORY

The `Poller` class appeared in MeasurementKit 0.1.0. It was renamed `Reactor` in
MeasurementKit 0.2.0. As of MK v0.2.0, the `Poller` still exists as a specific
implementation of the `Reactor` interface described in this manual page.
