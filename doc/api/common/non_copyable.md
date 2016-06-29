# NAME
NonCopyable &mdash; Forbids copyability

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

class NonCopyable {
  public:
    NonCopyable(NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &) = delete;
};

}
```

# DESCRIPTION

Forbids copyability. You should in general
forbid copyability when you hold low-level pointers that should
be `free()`d just once. To do this, just inherit from
`NonCopyable` as in:

```C++
/* Here inheriting from NonCopyable you guarantee that you cannot make
   by mistake copies of the raw pointer `bar`. Doing that would cause
   `delete` to be called more than once, which is a memory error. */
public class Foo : public mk::NonCopyable {
  public:
    Foo() {
        bar = new Bar;
    }
    
    ~Foo() {
        delete bar;
    }
  private:
    Bar *bar = nullptr;
};
```


# HISTORY

`NonCopyable` appeared in MeasurementKit 0.1.0.
