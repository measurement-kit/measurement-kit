# NAME

`measurement_kit/common/non_copyable.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_NON_COPYABLE_HPP
#define MEASUREMENT_KIT_COMMON_NON_COPYABLE_HPP

namespace mk {

class NonCopyable {
  public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &) = delete;
    NonCopyable() {}
};

} // namespace mk
#endif
```

# DESCRIPTION

`NonCopyable` makes a derived class non-copyable. You typically need to make non-copyable classes that manage the lifecycle of pointers. 

Available since measurement-kit v0.1.0.

