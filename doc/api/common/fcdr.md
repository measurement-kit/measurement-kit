# NAME
fcdr &mdash; Get from second to last element of a tuple

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename T, typename... U>
constexpr auto fcdr(std::tuple<T, U...> &&t);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fcdr` template function takes in input a non-empty tuple, removes
the first element, and returns the rest of the tuple.

The `fcdr` template SHOULD NOT compile if the tuple is empty.

Note that the `t` argument is passed by move, meaning that all the tuple
elements except the first one will be moved from the original tuple to the
result tuple. This is very efficient but prevents you from doing further
operations on the orginal tuple in most cases.

# EXAMPLE

```C++
REQUIRE(mk::fcdr(std::make_tuple(1, 2, 3)) == std::make_tuple(2, 3));
```

# HISTORY

The `fcdr` template function appeared in MeasurementKit 0.7.0.
