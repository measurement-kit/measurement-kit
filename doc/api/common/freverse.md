# NAME
freverse &mdash; Reverse of a tuple

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename... T> constexpr auto freverse(std::tuple<T...> &&t);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `freverse` template function takes in input a tuple and returns
back the reverse of such tuple.

The tuple is passed by move, meaning that tuple elements are moved from
the original tuple to the new tuple. This is very efficient, but also means
that you cannot continue using the original tuple in most cases.

# EXAMPLE

```C++
REQUIRE(mk::freverse(std::make_tuple(1, 2, 3)) == std::make_tuple(3, 2, 1));
```

# HISTORY

The `freverse` template function appeared in MeasurementKit 0.7.0.
