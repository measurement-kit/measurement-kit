# NAME
lexical_cast &mdash; Smart cast between different types.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename To, typename From> To lexical_cast(From f);

template <typename To, typename From> ErrorOr<To> lexical_cast_noexcept(From f);

}
```

# STABILITY
2 - Stable

# DESCRIPTION

The `lexical_cast` function template converts from scalar type `To` to scalar
type `From`, throwing `ValueError` if conversion could not be performed.

The `lexical_cast_noexcept` function template is like `lexical_cast` except that
it returns `ErrorOr<To>` rather than throwing exception.

The underlying implementation of `lexical_cast` uses `std::stringstream` to
convert back and forth from types; hence, the possible conversions are limited
to the ones that are implemented by `std::stringstream`'s `<<` and `>>`.

# EXAMPLE

```C++
#include <measurement_kit/common.hpp>

int main() {
    int num = lexical_cast<int>("1024");
    REQUIRE_THROWS(lexical_cast<int>("foobar"));
    ErrorOr<int> eo = lexical_cast_noexcept<int>("foobar");
    REQUIRE(!eo);
    REQUIRE_THROWS(*eo);
}
```

# BUGS

Since `std::stringstream` represents `true` using `1` and `false` using `0`, the
commonly established semantic that any nonzero value is `true` does not hold.

# HISTORY

The `lexical_cast` function template class appeared in MeasurementKit 0.2.0. There is
[a function template with similar functionality in
BOOST](http://www.boost.org/doc/libs/1_61_0/doc/html/boost_lexical_cast.html).
