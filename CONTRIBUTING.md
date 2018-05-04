# Contributing to Measurement Kit

Measurement Kit is a Free and Open Source software project under the BSD
license that welcomes new contributions.

In this document we explain how to contribute code to Measurement Kit and
what coding style you should follow for contributing.

## How to contribute code

Here we provide useful tips explaining you how to use git for contributing
and how the review process of a pull request should work.

### 1. Fork

You should first create a fork of Measurement Kit by clicking on the fork
button and cloning your fork with:

```
git clone git@github.com:$your_github_username/measurement-kit.git
cd measurement-kit
git remote add upstream https://github.com/measurement-kit/measurement-kit.git
```

### 2. Branch

Branches should be created from the `master` branch with an accurately
labelled name. For a feature branch, use `feature/short_feature_description`.
When working on a bugfix, use `fix/short_bug_description`.

Branch from master. Make sure your master is current before branching. You
can use the following commands:

```
git checkout master
git fetch upstream
git merge upstream/master
git checkout -b feature/short_feature_description  # or fix/...
```

Then you can start hacking.

### 3. Commit

Make sure git knows your username and email address with the following:

```
git config --global user.name "Jane Doe"
git config --global user.email "jane@example.com"
```

Try to keep the commits made on the branch small. A good branch should only
address one specific issue.

### 4. Test

Before submitting a pull request, make sure your unittests pass:

```
make check
```

If you add extra code or modify existing code, be sure that it is covered by
the existing unittests. If not write them.

In general, it would be good for pull requests not to reduce the current
code coverage of the project.

If you are submitting a branch fixing a bug, you should also be submitting a
unittest that is capable of reproducing the bug you are attempting to fix.

### 5. Open a Pull Request

You can then push your feature branch to your remote:

```
git push origin feature/short_feature_description  # or fix/...
```

Then you can open a pull request. Opening a pull request will start automatic
continuous integration tests. We should configure the continuous integration
system such that all warnings during the build are turned into errors. This is
to ensure that the contributed code is as clean as possible.

## Code Review process

Pull requests will be reviewed by a core developer, who will take responsibility
of the merge. A pull request will not be merged when continuous-integration do
not pass. There is an exception to this rule. We will merge a pull request with
failing continuous-integration tests when it is obvious that the failure is
caused by transient network errors unrelated with the pull request at hand. In
such cases, who is merging should explain why he thinks that the failing test or
tests are caused by transient network errors.

Core developers are also allowed to tag small, simple pull requests as
`hotfixes`. Such pull requests should be announced on some internal channel.
If no one objects to merge such pull request within one work day, the core
developer can then proceed with self merging this hotfix pull request.

If the diff is small, squash merge is preferred, otherwise preserve the history. 

Once a pull request is open and the review process is started, handle possible
conflicts with master by merging, resolving commits, and committing the merge so
that the reviewer can see how conflicts were resolved.

## Coding Style

Public headers should go in the `include/measurement_kit` folder. Private
headers should go in the `include/private` folder. Sources should go in
the `src/libmeasurement_kit` folder.

We use `clang-format` to indent sources automatically. If you are contributing
a new file, run `clang-format` before submitting a pull request. If you are
touching existing files, we will take care of formatting at a later time.

New contributions should use C++11. There is existing code in C++14, but we
should aim to have all the codebase in C++11, to support older systems.

In the following, we describe the preferred coding style. Existing code may not
fully conform to such description, but eventually it should be refactored to
conform.

### Public Headers

We have convenience public headers, like `<measurement_kit/common.hpp>`, and
fine grained headers, like `<measurement_kit/common/non_copyable.hpp>`.

The API is specified in terms of convenience public headers. The code inside
MK should use fine grained headers, to reduce compile time.

If your platform easily supports `iwyu`, consider periodically running it, in
order to reduce the include mess a little bit. But remember that it may also
suggest to include private `libc++` headers (don't!) and to replace including
specific headers with forward declarations (usually works).

### Namespaces

Everything inside `include/{private,measurement_kit}/common` and
`src/libmeasurement_kit/common` lives in the `mk` namespace.

All other submodules have their own private namespace. For example, code
in `src/libmeasurement_kit/dns` lives in the `mk::dns` namespace.

### Exceptions and Assertions

Try to avoid exceptions. Code that does not throw exceptions is more easy
to handle in asynchronous programs. To report errors:

1. return `Error` when you would instead return nothing and thrown an
   exception in case of failure;

2. return `ErrorOr<T>` when you would instead return `T` on success
   and thrown an exception in case of failure.

Both `Error` and `ErrorOr` are defined in `<measurement_kit/common.hpp>`.

It is okay to throw the following exceptions:

1. `std::bad_alloc` when out of memory;

2. `std::runtime_error` when an "impossible" condition occurs.

The library implementation should be such that exceptions will tear down the
current thread with the associated I/O loop, but do not affect any other
thread. Since we are moving towards the model where each test runs into its
own thread, this basically means than an exception (signalling a really
unexpected condition) will only cause the current test to fail.

Prefer exceptions to assertions. Assertions can be disabled and we do want
to have all the safety checks always in place. (But also try to ensure that
exceptions are not used in the fast path of performance-oriented tests;
sometimes it's very enlightening to disassemble the library and see how much
complexity is added by all the safety checks.)

### Move semantic

We use move semantic very often, usually to guarantee that a piece
of code has single ownership of a specific variable. In general,
we do not force move semantic in functions and methods signatures.
In other words, we write signatures like the following:

```C++
void do_something(std::string s);
```

But then, when it makes sense to move, we move:

```C++
do_something(std::move(s));
```

The only exception to this rule is `std::function`. In most cases we use
`std::function` to wrap a lambda function, with its closure. In such cases,
it is important that the lambda has unique ownership, especially in a
possibly-multi-thread context, such that the destructor of the closure
is then called only once. For this reason, in most function prototypes you
should see that we use `std::function &&` rather than just `std::function`:

```C++
void do_something(/* Args... */, std::function<void(/* Params... */)> &&func);
```

Using `&&` requires that the function is moved in. In most cases this would
work. In some cases, `std::move()` needs to be added explicitly.

### Raw Pointers and References

Try to avoid them as much as possible. Wrap raw pointers received by C in
convenience classes, to have RAII. Do not use pointers and references to share
values betwen classes. When something needs to be shared, use a shared
pointer instead. It is however okay to use references for passing a value
to a function that needs to modify it inplace.

### NonCopyable, NonMovable

Sometimes we have classes containing C pointers allocated with `malloc()` (or
similar). The destructors of such classes call `free()` (or similar). In these
cases, it is important to make the class non-copyable, otherwise `free()` would
be invoked multiple times. To this end, use `mk::NonCopyable` defined in
`"private/common/non_copyable.hpp"`:

```C++
class FooBar : NonCopyable {
    /* Definition... */
```

In many cases, you want the class to also be non movable. To this end, use
`mk::NonMovable` defined in `"private/common/non_movable.hpp"`. Be aware
that you need to use both `NonCopyable` and `NonMovable` to successfully make
a class both non copyable and non movable:

```C++
class FooBar : NonCopyable, NonMovable {
    /* Definition... */
```

As an alternative to `NonMovable`, you may want to implement move constructor
and move assignment to swap the C pointer with `nullptr` and so make the class
safely movable. Note that in this case you need also to handle the case where
the C pointer is `nullptr` in the destructor, and you also need to default
initialize the pointer to `nullptr`:

```C++
class Foobar : mk::NonCopyable {
  public:
    Foobar() : data{nullptr} {}

    Foobar(size_t count) {
        if ((data = malloc(count)) != 0) {
            throw std::bad_alloc();
        }
    }

    ~Foobar() {
        if (data != nullptr) {
            free(data);
        }
    }

    Foobar &operator=(Foobar &&other) {
        std::swap(data, other.data);
        return *this;
    }

    Foobar(Foobar &&other) { *this = std::move(other); }

    // Missing: other methods to use `data` (not relevant in this example)

  private:
    void *data;
};
```

Typically, we prefer making such classes both `NonCopyable` and `NonMovable`
and then we access them through shared or unique pointers.

### Auto

Avoid using `auto` unless the type is very long and convoluted. Listing the
type explicitly helps during code review.

### Template Magic

Do not use too much template magic. This makes the code too much complex for
people that does not code everyday in C++. Plus, it is very frustrating to
investigate template errors. Code heavily using templates (e.g.
`fcompose`) should be refactored to reduce the amount of template craziness.

### Smart Pointers

When we want to share structures and classes, we use smart pointers.
Currently, we mostly use shared pointers, because our most frequent
use case is that of capturing by copy a shared pointer inside a
lambda closure to keep the related struct or class alive _at least_
until the lambda has finished running.

Regarding shared pointers, please do not use `std::shared_ptr`
directly. Instead you should use the `mk::SharedPtr` replacement,
defined in `<measurement_kit/common.hpp>`.

This is a drop-in replacement for `std::shared_ptr` that makes sure
that an exception is thrown if the underlying pointer is `nullptr`.
We adopt this as an extra safety net for preventing `SIGSEGV`s
caused by programmer errors.

You should be able to use `mk::SharedPtr` whenever you would otherwise use the
standard library's `std::shared_ptr`. To make an `mk::SharedPtr`, the
preferred pattern is to call `mk::make_shared`:

```C++
    mk::SharedPtr<Foo> foo = mk::make_shared<Foo>();
```

This is like `std::make_shared`, except that it returns `mk::SharedPtr`
rather than `std::shared_ptr`.

### Lambda Closures

We use lambdas a lot. Especially, we use lambda closures to enforce
lifecycle constraints of collaborating objects. The following are the
recommended checks to perform in order to review lambda closures:

1. ideally, the lambda closure should list a single `mk::SharedPtr`
   instance pointing to a structure or class containing all the data
   you will need later when the lambda will be called. This is the
   ideal case because `mk::SharedPtr` uses an atomic counter to
   determine whether to destroy itself, and, moreover, because it
   will greatly simplify the review process;

2. if you are capturing many variables by copy or are using `=`, you
   should make sure of the following:

    2.1. that everything on the stack is either a `mk::SharedPtr` or
         a type that has no destructor (e.g. a `long long`);

    2.2. that, if you are copying in `std::function`, `std::string` or
         any other object with a destructor not listed above, the
         lambda will execute in the same thread context of the calling
         function (or the destructor might end up being called by
         different threads possibly causing hazards);

3. if you are capturing many variables by reference or are using `&`, you
   should make sure that the lambda will run in the current scope.

Much code uses `=` but, ideally, it would be best if it was refactored
according to rule 1. It is okay to use `&` in `main` when you are
referencing variables in the same scope of a blocking I/O loop call.

### Async Operations

There will be cases where a specific operation starts and cannot
be completed immediately but, rather, will complete at a later time.
In these cases, the function or method should have as its last
parameter a `std::function<T> &&` that will be called when the result
is available. The first parameter to such callback should be an
`Error`, indicating whether an error occurred, followed by additional,
optional parameters. Thus, in general, an async operation would have
a prototype similar to:

```C++
void async_operation(
        /* Params..., */
        std::function<void(Error /*, Callback params... */)> &&cb);
```

If any data structure is required by the async operation, we should
guarantee that such structure is alive until the end of the async
operation itself. In practice, this is mostly done through the above
mentioned `mk::SharedPtr<T>` shared pointer and lambda closures.
Typically this is implemented following this pattern:

```C++
struct Context {
    LowLevelContext llctx;
    std::function<void(Error /*, Params... */)> cb;
};

void async_operation(SharedPtr<Context> ctx /*, Params..., */,
        std::function<void(Error /*, Callback params... */)> &&cb) {
    if (!!ctx->cb) {
        cb(GenericError("already_pending") /*, Callback params... */);
        return;
    }
    // ...
    ctx->cb = std::move(cb);
    ll_async_operation(ctx->llctx /*, LL params... */,
            [ctx](Error err /*, LL callback params... */) {
                // ...
                std::function<void(Error /*, Callback params... */)> cb;
                std::swap(ctx->cb, cb);
                cb(err /*, Callback params... */);
            });
}
```

In case of immediate error (e.g. when there is an already pending
async operation), the callback may be invoked immediately to report
the error to the caller. This behavior is in general okay, except
when the current function is part of the public MK API (we explain
this case in detail below).

Before invoking the async operation, the callback should be stored
into the context structure. Above, we move the callback, rather
than copying it, which is more clear and correct: in general we
don't know whether the low level async operation will execute in
the same thread context. Thus, it is more robust to give the context
unique ownership of the callback.

The lambda passed to the async operation saves the context `ctx`
into its closure. This guarantees that `ctx` is alive _at least_
until the lambda is finished running, and possibly more. Note that
`SharedPtr` is based on `std::shared_ptr`, which uses an atomic
reference counter. Thus, there are no thread safety concerns caused
by copying `ctx` into a lambda managed by possibly another thread.

#### Thread Safety

In case you want to make the operation thread safe, add a
`std::recursive_mutex` to `Context` and remember to lock it:

```diff
 struct Context {
+    std::recursive_mutex mutex;
     LowLevelContext llctx;
     std::function<void(Error /*, Params... */)> cb;
 };

 void async_operation(SharedPtr<Context> ctx /*, Params..., */,
         std::function<void(Error /*, Callback params... */)> &&cb) {
+    std::unique_lock<std::recursive_mutex> _{ctx->mutex};
     if (!!ctx->cb) {
         cb(GenericError("already_pending") /*, Callback params... */);
         return;
     }
     // ...
     ctx->cb = std::move(cb);
     ll_async_operation(ctx->llctx /*, LL params... */,
             [ctx](Error err /*, LL callback params... */) {
+                std::unique_lock<std::recursive_mutex> _{ctx->mutex};
                 // ...
                 std::function<void(Error /*, Callback params... */)> cb;
                 std::swap(ctx->cb, cb);
                 cb(err /*, Callback params... */);
             });
 }
```

We prefer `std::recursive_mutex` to `std::mutex`, because it protects us
from the case where the callback of the operation is called immediately
and the code of the callback calls the operation again. That will result
in a deadlock with a normal `std::mutex`.

#### Persistent Polling

In some cases, a callback may be called multiple times, and you need to
explicitly tell the code when you want to stop receiving the event(s) for
which you registered a callback. To this end, add a function for telling
MK to stop receiving the event(s) and adjust the code to deal with the
case whether the user wants to continue receiving event(s):

```diff
 void start_operation(SharedPtr<Context> ctx /*, Params..., */,
         std::function<void(Error /*, Callback params... */)> &&cb) {
     if (!!ctx->cb) {
         cb(GenericError("already_pending") /*, Callback params... */);
         return;
     }
     // ...
     ctx->cb = std::move(cb);
     ll_async_operation(ctx->llctx /*, LL params..., */,
             [ctx](Error err /*, LL callback params... */) {
                 // ...
                 std::function<void(Error /*, Callback params... */)> cb;
                 std::swap(cb, ctx->cb);
                 cb(err /*, Callback params... */);
+                if (!ctx->cb) {
+                    std::swap(cb, ctx->cb);
+                }
             });
 }
+
+void stop_operation(SharedPtr<Context> ctx) { ctx->cb = nullptr; }
```

Note that, above, we have named the functions `start_{operation}` and
`stop_{operation}`, to convey more clearly the meaning that the callback
will be called more then once and you have to tell MK when you don't
want to receive more events.

#### Async Methods

So far we only have discussed functions accessing structures kept alive
using `mk::SharedPtr`. However, since C++ is object oriented, you may
want to work with objects. In such case, async methods should be static
and should receive as their first argument a `mk::Shared_ptr`:

```C++
class AsyncObject {
  public:
    static void async_operation(SharedPtr<AsyncObject> ctx /*, Params..., */,
        std::function<void(Error /*, Callback params... */)> &&cb);

    Error sync_setter(std::string key, std::string value);

    ErrorOr<std::string> sync_getter(std::string key);

  private:
    LowLevelContext llctx;
    std::function<void(Error /*, Params... */)> cb;
};
```

Of course, non-async methods do not need to be static. In general, as for most
Measurement Kit code, it would be better if they would not throw exceptions,
as this simplifies writing reliable async code.

### C Callbacks

You should declare C callbacks as `extern "C"` functions, because that is the
most portable way of interfacing C and C++, according to C++'s FAQ.

### Coding for Testability

We use specific templates and macros to make our code more testable. A class,
or a free function, that depends on specific lower level APIs should be a
template depending on such functions. This allows you to easily write test
cases where such dependencies fail. The process of writing the templates
is simplified by the `MK_MOCK` and `MK_MOCK_AS` macros defined
in `private/common/mock.hpp`. For example:

```C++
template <MK_MOCK_AS(kqueue, sys_kqueue)>
class KqueueHolder : NonCopyable, NonMovable {
  public:
    ~KqueueHolder() {
        if (kfd != -1) {
            (void)close(kfd);
            kfd = -1;
        }
    }

    KqueueHolder() {
        if ((kfd = sys_kqueue()) < 0) {
            throw std::runtime_error("kqueue_failed");
        }
    }

  private:
    int kfd = -1;
};
```

In this example, we use `MK_MOCK_AS` to declare a template parameter named
`sys_kqueue`, with the same type of `kqueue(2)`, and by default pointing
to `::kqueue`. That is, when constructed as `KqueueHolder<>`, this template
class will use the system's `kqueue` implementation. But we can easily
construct a regress test -- using `philsquared/Catch` syntax -- as follows:

```C++
static inline int kqueue_fail() { return -1; }

TEST_CASE("KqueueHolder works") {
    REQUIRE_THROWS(KqueueHolder<kqueue_fail>{});
}
```

This basically creates another instance of the template, where the template
parameter is an always failing inline kqueue-like function rather than the
official `::kqueue` from the system. This allows us to verify that, when
`::kqueue` fails, we indeed throw an exception.

### Code Organization

We prefer to create a single file for class, or functionality, named like the
class, or functionality, all lowercase and using snake case.

In general, we have a private header
`include/private/${module}/${snake_case_name}.hpp` that defines
one, or more, template classes or functions instrumented for
testability.

Then, we have code in
`src/libmeasurement_kit/${module}/${public_api_name}.cpp` that
implements public APIs defined in
`include/measurement_kit/${module}/${public_api_name}.hpp` using
the default instantiation of the template(s) defined in private
headers.

Finally, we have unit and regress tests in
`test/${module}/${snake_case_name}.cpp` that specialize template(s)
differently for testability.

Integration tests (i.e. tests that really use the network) should
be protected by `#ifdef ENABLE_INTEGRATION_TESTS`.

### Modules API Layout

We prefer to avoid to expose state in modules APIs, unless there is a clear
need to do so. For this reason, many modules APIs are async functions following
this pattern:

```C++
MK_API(void) module_level_api(
        /* Arguments... */,
        Settings settings,
        SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger,
        std::function<void(Error /*, Parameters... */)> &&cb);
```

Settings must be copied, because each API function should have its private
copy to mess with. Reactor is the object managing the I/O loop; many module
APIs assume that the I/O loop is already running. Logger is the object
controlling whether and how logs are printed. The callback will be invoked
when the requested async operation is completed.

Internally, the module API will typically:

- construct a `mk::SharedPtr<State>` object or class taking in input by
  copy or move all the parameters passed to the module API;

- defer execution in the next I/O loop, using `reactor->call_soon`, to
  guarantee that the callback will always be called after the current
  function has finished running

- pass the afore mentioned `mk::SharedPtr` in the closure of the
  `reactor->call_soon` lambda, to use it later.

For example:

```C++
MK_API(void) module_level_api(
        /* Arguments... */,
        Settings settings,
        SharedPtr<Reactor> reactor,
        SharedPtr<Logger> logger,
        std::function<void(Error /*, Parameters... */)> &&cb) {
    mk::SharedPtr<State> state{mk::make_shared<State>(
            /* Moving arguments... */,
            std::move(settings),
            reactor,
            std::move(logger),
            std::move(cb))};
    reactor->call_soon([state]() { State::async_op(state); });
}
```

Note that the reactor cannot be moved when construcing `State` because
it must also be used later to call `call_soon`.

### Code documentation

Just write descriptive comments before the pieces of code you want to
document, without leaving empty lines between documentation and the
related pieces of code.

### Interaction with C code

Here we discuss interaction with C code by providing a complete
example integrating most concepts discussed above.  We will create
a class for holding a libevent's `bufferevent` created by other
code.

```C++
// new file include/private/net/bev_holder.hpp

#ifndef PRIVATE_NET_BEV_HOLDER_HPP
#define PRIVATE_NET_BEV_HOLDER_HPP

#include "private/common/mock.hpp"                // for MK_MOCK
#include "private/common/non_copyable.hpp"        // for NonCopyable
#include "private/common/non_movable.hpp"         // for NonMovable
#include <event2/bufferevent.h>                   // for bufferevent
#include <functional>                             // for function
#include <measurement_kit/common/shared_ptr.hpp>  // for SharedPtr
#include <mutex>                                  // for recursive_mutex

extern "C" {

static inline void do_event(bufferevent *, short, void *);

static inline void do_read(bufferevent *bev, void *opaque) {
    do_event(bev, BEV_EVENT_READING, opaque);
}

static inline void do_write(bufferevent *bev, void *opaque) {
    do_event(bev, BEV_EVENT_WRITING, opaque);
}

} // extern "C"

namespace mk {
namespace net {

// `BevHolder` holds a bufferevent created by other code.
//
// It is non copyable and non movable because it passes `this` to libevent code.
//
// All methods are thread safe.
template <MK_MOCK(bufferevent_enable), MK_MOCK(bufferevent_disable)>
class BevHolder : NonCopyable, NonMovable {
  public:
    // The `BevHolder` constructor initializes this class with an already
    // allocated bufferevent structure pointer. Note: this code assumes
    // you have set the `BEV_OPT_CLOSE_ON_FREE` bufferevent flag.
    BevHolder(bufferevent *bev) : bev{bev} {}

    // The `~BevHolder` destructor frees the bufferevent.
    ~BevHolder() {
        if (bev != nullptr) {
            bufferevent_free(bev);
        }
    }

    // The `start_poll` static method starts polling a bufferevent for I/O.
    //
    // The first parameter is a shared pointer to a `BevHolder`. This
    // is a shared pointer so that this static method could take ownership
    // of the `BevHolder` until polling is in progress.
    //
    // The second parameter is a callback called repeatedly to update on the
    // polling state of the bufferevent. This callback takes as argument a
    // bitmask of flags operations returned by bufferevent:
    //
    // - BEV_EVENT_READING means that we are reading
    // - BEV_EVENT_WRITING means that we are writing
    //
    // If both are set, we are both reading and writing. If both are not set,
    // we don't know whether we're reading or writing (this usually indicates
    // a bufferevent filter).
    //
    // The above flags are or'ed with the following error conditions:
    //
    // - BEV_EVENT_ERROR means socket or SSL error
    // - BEV_EVENT_EOF means read EOF
    // - BEV_EVENT_TIMEOUT means there was a timeout
    //
    // If no error flag is set, the operation was successfully.
    //
    // If polling is already in progress, the callback will be called
    // immediately with flags equal to BEV_EVENT_ERROR.
    //
    // If it is not possible to enable read, a std::runtime_error is thrown.
    static void start_poll(SharedPtr<BevHolder> self,
            std::function<void(short)> &&cb) {
        std::unique_lock<std::recursive_mutex> _{self->mutex};
        if (!!self->ref) {
            // If it's already set, then we're already polling
            cb(BEV_EVENT_ERROR);
            return;
        }
        if (bufferevent_enable(self->bev, EV_READ) != 0) {
            throw std::runtime_error("bufferevent_enable_failed");
        }
        self->callback = std::move(cb);
        bufferevent_setcb(self->bev, do_read, do_write, do_event, self.get());
        self->ref = self;
    }

    // The `stop_poll` method stops polling for I/O.
    //
    // Throws std::runtime_error if it's not possible to disable reading
    // on the bufferevent.
    void stop_poll() {
        std::unique_lock<std::recursive_mutex> _{mutex};
        if (bufferevent_disable(bev, EV_READ) != 0) {
            throw std::runtime_error("bufferevent_disable_failed");
        }
        callback = nullptr;
        bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
        ref = nullptr;
    }

    // The `bev_callback_` semi-hidden method is called by bufferevent's
    // code when there is an update on the bufferevent state.
    //
    // The first argument is the flag of events that occurred, as described
    // above in start_poll() documentation.
    //
    // Since this method is thread safe, you don't need to set the
    // `BEV_OPT_SERIALIZE_CALLBACKS` flag on the bufferevent.
    void bev_callback_(short evt) {
        std::unique_lock<std::recursive_mutex> _{mutex};
        std::function<void(short)> cb;
        std::swap(cb, callback);
        cb(evt);
        if (!!ref && !callback) {
            // If ref is set, either we didn't `stop_poll` or we `stop_poll`-ed
            // and then we `start_poll`-ed. In the former case, the callback
            // is still `nullptr` from the swap above and must be set
            // again. In the latter case, on the contrary, the callback
            // has been changed by `start_poll` and should not be touched.
            std::swap(cb, callback);
        }
    }

    // Note: all public for simplicty (this is a private class)

    // Self reference used to keep alive the object while used by libevent.
    SharedPtr<BevHolder> ref;

    // Pointer to the bufferevent structure.
    bufferevent *bev;

    // Callback to be called to inform user about state changes.
    std::function<void(short)> callback;

    // Mutex providing thread safety.
    std::recursive_mutex mutex;
};

} // namespace net
} // namespace mk

static inline void do_event(bufferevent *, short evt, void *opaque) {
    using namespace mk::net;
    using namespace mk;
    // This reference keeps alive the object until end of scope also in
    // the case in which `stop_poll` is called and `->ref` is cleared.
    SharedPtr<BevHolder<>> ref = static_cast<BevHolder<> *>(opaque)->ref;
    ref->bev_callback_(evt);
    // FIXME: libevent v2.0.x crashes with filter bufferevents if we destroy
    // a bufferevent that is currently in use. Here, if `stop_poll` has
    // been called, the bufferevent would exit out of scope. But it's clearly
    // in use, since we have been called by bufferevent code.
}

#endif

// modified file src/libmeasurement_kit/net/transport.cpp

// TODO: modify use the above header

// new file test/net/bev_holder.cpp

// TODO: use the above header

```

The example has touched upon most of the aspects discussed above. Here the main
difference is that it is more complex to keep the object alive, because below
on the stack we don't have a C++ object but libevent's C code. So, to keep the
object alive, we add a self-referencing field, that is set to the shared pointer
keeping safe the `BevHolder` when polling, and cleared when not polling.

This is somewhat related with actual MK code, which is more complicated, deals
with more edge cases (including the `FIXME` comment), and is more tested. In
many cases, your code will be simpler than this, because you would have a lower
level C++ object to which to pass a lambda callback keeping alive the object
you are currently working on.
