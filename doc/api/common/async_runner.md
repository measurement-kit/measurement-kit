# NAME
AsyncRunner &mdash; runs tasks in a background I/O thread

# LIBRARY
MeasurementKit (libmeasurement\_kit, -lmeasurement\_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class AsyncRunner {
  public:
    template <typename Task, typename Callable>
    void start(std::string &&name, Var<Logger> logger,
               Task &&task, Callback &&callback) {
        // Roughly equivalent to (but more robust than):
        task([=](const Error &&err) {
            callback(err);
        });
    }

    ~AsyncRunner();

    void stop();

    long long active();

    bool running();

    Var<Reactor> reactor();

    static Var<AsyncRunner> make();

    static Var<AsyncRunner> global();
};

} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION


The `AsyncRunner` class runs schedules deferred execution of specific tasks
in a background I/O thread where a `Reactor` I/O loop is running.

The `start` method is the most important method of this class. It performs the
following operations:

1. it schedules the deferred execution of `task` on a background thread where
   a I/O `Reactor` loop is running;

2. on the background I/O thread, executes `task` and captures its result;

3. passes such result to `callback`;

4. logs all operations under the task `name` and using `logger`.

The `task` argument is a generic callable equivalent to
`Continuation<Error>`. That is, `task` is a function taking
a `Callback<Error>` in input. When done, `task` MUST call
such `Callback<Error>` to pass it the final result.

The `callback` argument is a generic callable equivalent
to `Callback<Error>`. It is called with the result of `task`.

The following snippet demonstrates the expected usage:

```C++
/*
 * With lambda based wrappers:
 */
Var<Task> task = Var<State>::make();
AsyncRunner::global()->start(
    "dummy task", Logger::global(),
    [task](Callback<Error &&> &&cb) {
        task->do_async_operation(std::move(cb));
    },
    [](const Error &&error) {
        // Execute code after the task is complete
    });

/*
 * With callable objects
 */
class Task {
  public:
    void operator()(Callback<Error> &&cb) {
    }

    // ...
};

AsyncRunner::global()->start(
    "another task",
    Logger::global(),
    Task{},
    [](const Error &&error) {
        // Execute code after the task is complete
    });
```

The `task` passed as third argument is garbage collected by this method,
such that it will not be destroyed immediately after the callback it receives
as its first argument is called. Rather its destruction will be deferred to
occur as part of a "call soon" scheduled in the `AsyncRunner`'s `Reactor`.

This guarantees that the stack of functions that led to producing the result
of `task` can unwind safely without causing any use after free hazard. See
the GUARANTEES section for more information.

The `~AsyncRunner` destructor calls `stop`.

The `stop` method stops the runner I/O loop and terminates all the tasks
that are currently running, without calling their callbacks. It waits for
the background thread to terminate before continuing.

The `active` method returns the number of tasks currently active.

The `running` method tells you whether the background thread is running.

The `reactor` method returns the reactor in use. If the task needs to use a
specific reactor, you SHOULD pass it the one returned by this method.

The `make` factory constructs a shared instance of this object.

The `global` factory returns the global shared `AsyncRunner`.

# GUARANTEES

The `start` method provides the following guarantees:

1. the fourth argument `callback` will not be called immediately but
   rather it will be deferred using `call_soon`;

2. the third argument `task` will be alive until the `callback` passed
   as fourth argument returns.

The combination of this two guarantees avoids the following use after
free hazard that we observed empirically:

1. as it is currently implemented, `start` manages the lifecycle of `task`
   using the closure of the function to be passed to `task`;

2. as such, unless there are other references to `task` around, `task`
   will be destroyed just after the function passed to it returns;

3. yet, `task` implementation may be such that, below the stack frame where
   the final callback is called, there are other uses of either `task` or
   objects whose lifecycle is managed by task;

4. as such, without the two following guarantees, when the stack frame is
   unwinding after the final callback has been called, it MAY be that there
   is code using already-deleted objects.

# CAVEATS

Since the fourth-argument `callback` will be called from a background
thread, make sure you lock any shared resources before proceeding:

```C++
AsyncRunner::global()->start(
    "dummy task", Logger::global(),
    [](Callback<Error &&> &&) {
        // SOMETHING...
    },
    [](Error &&error) {
        // LOCK SHARED RESOURCES BEFORE PROCEEDING
    });
```

# BUGS

Altough it is a template, `start` does not allow for arbitrary arguments.

# HISTORY

The `AsyncRunner` appeared in MK v0.7.0. It is a generalized version
of `nettests::Runner` that works on generic tasks.
