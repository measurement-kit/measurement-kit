# NAME

`measurement_kit/common/settings.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_SETTINGS_HPP
#define MEASUREMENT_KIT_COMMON_SETTINGS_HPP

namespace mk {

class Settings : public std::map<std::string, Scalar> {
  public:
    using std::map<std::string, Scalar>::map;

#define XX(_rv_, _methname_, _accessor_)                                       \
    template <typename Type>                                                   \
    _rv_ _methname_(std::string key, Type def_value) const {                   \
        if (find(key) == end()) {                                              \
            return def_value;                                                  \
        }                                                                      \
        return at(key)._accessor_<Type>();                                     \
    }

    XX(Type, get, as)

    XX(ErrorOr<Type>, get_noexcept, as_noexcept)

#undef XX

  protected:
  private:
};

} // namespace mk
#endif
```

# DESCRIPTION

`Settings` maps a key string to a Scalar value. This class is used throughout MK to pass around settings. We generally pass Settings around by value, so each function has its private copy. 

Settings is a derived class of std::map<std::string, Scalar>. As such it has all the methods of a standard std::map. 

Available since measurement-kit v0.1.0. 

Support for mapping from string to any scalar type (rather than to just strings) was added in MK v0.2.0.

`get()` returns the specified key if set; otherwise it returns the default value def_value. Throws ValueError if the value associated to key cannot be converted to the specified type.

`as_noexcept` is like `as()` but return Error rather than throwing.

