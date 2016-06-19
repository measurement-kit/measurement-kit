# NAME
Callback -- Syntactic sugar for writing callbacks.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template<typename... T> using Callback<T...> = std::function<void(T...)>;

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `Callback` alias allows to more compactly writing callbacks and SHOULD be
used to indicate one-shot callbacks instead of `std::function<T>`.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

using namespace mk;

static void slow_operation(Callback<Error> cb) {
    debug("slow operation ...");
    call_later(1.0, [=]() {
        debug("slow operation ... done");
        cb(NoError());
    });
}

int main() {
    loop_with_initial_event([=]() {
        set_verbose(MK_LOG_DEBUG);
        slow_operation([=]() {
            break_loop();
        });
    });
}
```

# HISTORY

The `Callback` alias appeared in MeasurementKit 0.2.0.
