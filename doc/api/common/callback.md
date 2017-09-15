# NAME

`measurement_kit/common/callback.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_CALLBACK_HPP
#define MEASUREMENT_KIT_COMMON_CALLBACK_HPP

namespace mk {

template <typename... T> using Callback = std::function<void(T...)>;

} // namespace mk
#endif
```

# DESCRIPTION

`Callback` is syntactic sugar for writing callback functions. In general, we try to use `Callback` in the code to name functions that will be called _after_ the function to which they have been passed has returned. That is, the function is supposed to schedule their deffered execution but should not execute it directly. 

The `Callback` alias was added in measurement-kit v0.2.0.

