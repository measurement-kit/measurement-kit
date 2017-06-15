# NAME
fmap &mdash; apply a function to a vector.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

template <typename A, typename B>
std::vector<B> fmap(std::vector<A> i, std::function<B(A)> f);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `fmap` function applies `f` to every element in `i` and returns back
the corresponding vector.

# HISTORY

The `fmap` function appeared in MeasurementKit 0.3.0.
