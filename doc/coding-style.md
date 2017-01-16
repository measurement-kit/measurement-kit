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

To represent a function to be called later, we use the `Callback` template,
which actually is just an alias for

```C++
template <typename... T> using Callback = std::function<void(T...)>;
```

so you can write `Callback<Foo, Bar>` as opposed to
`std::function<void(Foo, Bar)>`, which is not only shorter but it also
express clearly that the function is expected to be called sometime
later (and typically not directly by the function to which it is passed).

For example,

```C++
/// Calls the callback after timeout seconds have passed.
/// \param timeout Number of seconds of delay.
/// \param callback Callback.
/// \note This function implicitly uses the default poller.
void call_later(double timeout, Callback<> callback);
```

Internally, this could be implemented using a closure object (1) created when
`call_later()` is called, (2) used to hold `timeout`, `callback` and other bits
of state needed to implement `call_later()`, and (3) destroyed when `callback`
has been called.

That is,

```C++
class CallLater {
  public:
    CallLater(double t, Callback<> f);
    void handle_cb();

  private:
    double timeout;
    Callback<> cb;
    event_system_handle_t *call = nullptr;

    ~CallLater();
};

extern "C" {
static void c_callback(void *p) {
    static_cast<CallLater *>(p)->handle_cb();
}
} // extern "C"

CallLater::CallLater(double t, Callback<> f) {
    timeout = t;
    cb = f;
    call = event_system_call_later(t, c_callback, this);
}

void CallLater::handle_cb() {
    cb();
    delete this;
}

CallLater::~CallLater() {
    event_system_free_handle(call);
}

void call_later(double timeout, Callback<> callback) {
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
a library user) is to just expose the `call_later` function. In fact, being
this approach abstract, we can decide to implement it also in the
following, more compact way:

```C++
struct CallLater {
    Callback<> callback;
    event_system_handle_t *call = nullptr;
};

extern "C" {
static void c_callback(void *p) {
    auto context = static_cast<CallLater *>(p);
    context->callback();
    event_system_free_handle(context->call);
    delete context;
}
} // extern "C"

void call_later(double timeout, Callback<> callback) {
    auto context = new CallLater;
    context->callback = std::move(callback);
    context->call = event_system_call_later(timeout, c_callback, context);
}
```

The advantage of this second version of the code is that it is more compact,
and hence that would be our preferred choice.
