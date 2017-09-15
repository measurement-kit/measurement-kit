# NAME

`measurement_kit/common/scalar.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_SCALAR_HPP
#define MEASUREMENT_KIT_COMMON_SCALAR_HPP

namespace mk {

class Scalar : public std::string {
  public:
    Scalar() {}

    template <typename Type> Scalar(Type value) {
        std::stringstream ss;
        ss << value;
        assign(ss.str());
    }

    template <typename Type> Type as() const {
        std::stringstream ss{c_str()};
        Type value{};
        ss >> value;
        if (!ss.eof()) {
            throw ValueError(); // Not all input was converted
        }
        if (ss.fail()) {
            throw ValueError(); // Input format was wrong
        }
        return value;
    }

    template <typename Type> ErrorOr<Type> as_noexcept() const noexcept {
        try {
            return as<Type>();
        } catch (const Error &e) {
            return e;
        }
    }

  protected:
  private:
};

} // namespace mk
#endif
```

# DESCRIPTION

`Scalar` wraps a scalar value. Internally we use std::stringstream to convert the scalar value to a string. This allows us to access the original scalar value later. See lexical_cast() for a discussion of converting between scalar values using std::stringstream. 

Available since measurement-kit v0.2.0. 

Before MK v0.8.0 Scalar was named SettingsEntry.

The default constructor constructs an empty scalar. This basicall means that internall we will store an empty string.

The constructor with type constructs a scalar of the specified type with the specified value. Parameter value the value with which to initialize the scalar.

`as()` converts the scalar into the specified type. Throws ValueError if the conversion is not possible. Returns the converted value otherwise.

`as_noexcept()` is like except but, rather than throwing on error, returns the error that occurred.

