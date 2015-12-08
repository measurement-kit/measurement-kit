# NAME
DelayedCall -- A function to be called later.

# SYNOPSIS
```C++
#include "src/common/delayed_call.hpp"

using namespace mk;

// Create empty delayed call object
DelayedCall call;

// Initialize `call` with pending delayed call
call = DelayedCall(1.75, []() {
    // Do someting
});
```

# DESCRIPTION

`DelayedCall` represents a delayed call (i.e. a function to
be called later by the I/O loop). The default constructor creates
an empty delayed call. Or you can construct providing the delay
and the function to be called later. The result of calling the
constructor can be assigned to a previously existing delayed call
object, a demonstrated above.

To bind the delayed call lambda to a specific context you can
use the capture list. It is recommended to provide to the capture
list only references and pointers (we experienced issues using
the capture lists for keeping objects alive). In particular, when
a delayed call belongs to a class, it is recommended to:

1. declare the delayed call as a class variable so that it has
   the same lifecycle of the class

2. provide `this` to the capture list to route events from
   the delayed call to the container class

For example:

```C++
class Foo {
  public:
    Foo() { call = DelayedCall(1.75, [this]() { bar(); }); }
    void bar() { /* do something */ }
  private:
    DelayedCall call;
};
```

A similar recommendation applies when you register delayed calls
in the `main()` scope. In such case, just ensure that delayed calls
will have the same lifecycle of `main()`. For example:

```C++
int main() {
    DelayedCall call;
    call = DelayedCall(1.75, []() { measurement_kit::break_loop(); });
    measurement_kit::loop();
}
```

# HISTORY

The `DelayedCall` class appeared in MeasurementKit 0.1.0.
