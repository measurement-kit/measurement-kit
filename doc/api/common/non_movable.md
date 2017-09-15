# NAME

`measurement_kit/common/non_movable.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_NON_MOVABLE_HPP
#define MEASUREMENT_KIT_COMMON_NON_MOVABLE_HPP

namespace mk {

class NonMovable {
  public:
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
    NonMovable() {}
};

} // namespace mk
#endif
```

# DESCRIPTION

`NonMovable` makes a derived class non-movable. You typically need to make non-movable classes that manage the lifecycle of pointers. 

Appeared in measurement-kit v0.1.0.

