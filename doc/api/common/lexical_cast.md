# NAME

`measurement_kit/common/lexical_cast.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```
#ifndef MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP
#define MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP

namespace mk {

template <typename To, typename From> To lexical_cast(From f) {
    return Scalar{f}.as<To>();
}

template <typename To, typename From>
ErrorOr<To> lexical_cast_noexcept(From f) {
    return Scalar{f}.as_noexcept<To>();
}

} // namespace mk
#endif
```

# DESCRIPTION

`lexical_cast()` converts from one scalar type to another. If the conversion could not be performed, it throws ValueError. 

Internally, lexical_cast uses a std::stringstream to perform the conversion from the original type to a string and then from a string into the derived type. Precision may be lost in the conversion. Also, of course, conversion will be limited by what std::stringstream can do by default. 

_BUG_: std::stringstream represents `true` (the boolean value) as the string `"1"` and `false` as `"0"`. So, the string `"true"` won't probably be correctly converted as a bool value. This also means that the well-established semantic that non-zero is true won't work. 

_BUG_: old versions of lexical_cast() could not correctly convert the empty string back to an empty string. 

This functionality is mainly used to convert input settings from any type to string for later processing by MK. It is also handy when processing and converting command line arguments. 

Returns the converted value provided that conversion is possible. 

Throws ValueError if conversion is not possible. 

Appeared in measurement-kit v0.2.0.

`lexical_cast_noexcept` is like lexical_cast() except that it returns, rather than throwing, any error that may occur. 

Appeared in measurement-kit v0.2.0.

