# NAME
SettingsEntry &mdash; Generic scalar type

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
namespace mk {

class SettingsEntry : public std::string {
  public:
    template <typename In> SettingsEntry(In value);
    template <typename Out> Out as();
    template <typename Out> ErrorOr<Out> as_noexcept();
};

}
```

# STABILITY

1 - Experimental

# DESCRIPTION

`SettingsEntry` holds a generic scalar type. Under the hood, `SettingsEntry`
is a string and `std::stringstream` is used to convert to/from types; this is
similar to what we also do with `mk::lexical_cast<From, To>()`.

The constructor takes in input a scalar type and internally serializes it
as a string. Conversion errors should be detected at compile time, hence this
method should not throw any exception.

The `as()` method allows to convert the internally serialized value to
the specified `Out` type. This would raise `ValueError` if either conversion
is not possible or not all the internally serialized string is consumed
by the conversion.

The `as_noexcept()` method is just like `as()` except that it returns `ErrorOr<Out>`
rather than throwing exception on error.

# BUGS

Since `std::stringstream` represents `true` using `1` and `false` using `0`, the
commonly established semantic that any nonzero value is `true` does not hold.

# HISTORY

The `SettingsEntry` class appeared in MeasurementKit 0.2.0.
