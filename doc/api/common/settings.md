# NAME
Settings &mdash; Map from string to scalar type

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
namespace mk {

class Settings : public std::map<std::string, SettingsEntry> {
  public:
    using std::map<std::string, SettingsEntry>::map;
    template <typename T> T get(std::string key, T defval);
    template <typename T> ErrorOr<T> get_noexcept(std::string key, T defval);
};

}
```

# STABILITY

1 - Experimental

# DESCRIPTION

`Settings` is an extended `std::map` able to map string to any scalar type. We
use `SettingsEntry` to implement a generic scalar type container.

In addition to the typical methods of `std::map`, `Settings` also features
the `get()` and the `get_noexcept()` method.

The `get()` method returns the value
at `key` if available, otherwise the default value `defval`. An exception may
be raised if it is not possible to convert the value associated to `key`
to type `T`.

The `get_noexcept()` method is like `get()` except that it does
not throw exceptions and instead it returns a `ErrorOr<T>` type.

# HISTORY

The `Settings` class appeared in MeasurementKit 0.1.0. Support for mapping to
any type was added in MeasurementKit 0.2.0.
