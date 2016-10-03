# Coding style

This document details coding style issues.

## Formatting source code

As regards the way in which code should be formatted, we use `clang-format` for
this task. Just run:

```bash
clang-format -i $files
```

to have `$files` formatted according to the project's style.

## Code patterns

We organize similar bits of code using specific patterns.

### Deferred operations

In measurement-kit we have several places where a slow operation must be
called and its result (or error) must be processed after some time.

One such example is the delayed function call, where the programmer wants a
function to be called after a specified amount of time.

Another, similar example is the DNS query, where the programmer wants to
resolve a domain name to one, or more, IP addresses.

In these cases, and in similar cases, measurement-kit API must expose
a function that receives the requested parameters and calls a function
when the operation is complete.

For example,

```C++
/// Calls the callback after timeout seconds have passed.
/// \param timeout Number of seconds of delay.
/// \param callback Callback.
/// \note This function implicitly uses the default poller.
void call_later(double timeout, std::function<void()> callback);
```

Internally, this should be implemented using a closure object (1) created when
`call_later()` is called, (2) used to hold `timeout`, `callback` and other bits
of state needed to implement `call_later()`, and (3) destroyed when `callback`
has been called.

That is,

```C++
class CallLater {
  public:
    double timeout;
    std::function<void()> cb;

    CallLater(double t, std::function<void()> f) {
        timeout = t;
        cb = f;
        // TODO: configure libevent to call `handle_cb` after `t` seconds
    }

    void handle_cb() {
        cb();
        delete this;
    }

  private:
    ~CallLater() {
        // TODO: destroy any libevent state created by constructor
    }
};

void call_later(double timeout, std::function<void()> callback) {
    new CallLater(timeout, callback);
}
```

The structure of the implementation should help to understand why we do not
expose `CallLater` directly and instead only expose `call_later()`.

The destructor of `CallLater` shall be private because the closure shall
delete itself when it is complete, hence better to prevent the programmer
to call the destructor herself.

But then, assuming we expose `CallLater`, to issue a delayed call the
programmer would need to write this:

```C++
    new DelayedCall(3.14, []() {
        // Do something
    });
```

Which is kind of strange because you allocate on the heap an object that
has the ownership of itself, whose pointer you cannot delete, and that has
no other methods that you could call. Once it is clear how the code works
this is OK, but for casual hackers reading the code it may be confusing.

To make the above less confusing we could restructure the code to work
differently and require the programmer to write:

```C++
    new DelayedCall(3.14, [](DelayedCall *call) {
        // Do something
        delete call;
    });
```

But that's annoying because the programmer needs to remember to delete the
delayed call object. Since this is something to be done in any case, it would
probably be better to let the library do that.

So, we concluded that the most abstract approach (and the simpler one for
a library user) is to just expose the `call_later` function.
