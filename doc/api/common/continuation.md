# NAME
Continuation -- Syntactic sugar for functions that resume a paused function.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The Continuation alias allows to more compactly express a function that
resumes another function that has paused waiting for explicit continuation.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

using namespace mk;

static void coroutine(Callback<Error, Continuation<Error>> cb) {
    debug("initial slow operation ...");
    call_later(1.0, [=]() {
        debug("initial slow operation ... done");
        cb(NoError(), [=](Callback<Error> cb) {
            debug("other slow operation ...");
            call_later(1.0, [=]() {
                debug("other slow operation ... done");
                cb(NoError());
            });
        });
}

int main() {
    loop_with_initial_event([=]() {
        set_verbose(MK_LOG_DEBUG);
        debug("call coroutine ...");
        coroutine(Error err, [=](Continuation<Error> cc) {
            debug("call coroutine ... done");
            debug("slow operation in main ... ");
            call_later(1.0, [=]() {
                debug("slow operation in main ... done");
                debug("resume coroutine ...");
                cc([=](Error err) {
                    debug("resume coroutine ... done");
                    break_loop();
                });
            });
        });
    });
}
```

# HISTORY

The `Continuation` alias appeared in MeasurementKit 0.2.0.
