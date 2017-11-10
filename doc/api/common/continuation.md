# NAME

`measurement_kit/common/continuation.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_CONTINUATION_HPP
#define MEASUREMENT_KIT_COMMON_CONTINUATION_HPP

namespace mk {

template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

} // namespace mk
#endif
```

# DESCRIPTION

`Continuation` is a function that will call another function when complete. In several places we need to return code to be executed later, perhaps in parallel with other code. In such cases, we usually return a function that captures relevant parameters and receives in input a callback to be called later when done. The `Continuation` alias allows to express the returned function more compactly. 

Available since measurement-kit v0.2.0.

