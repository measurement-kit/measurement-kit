// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SETTINGS_HPP
#define MEASUREMENT_KIT_COMMON_SETTINGS_HPP

#include <measurement_kit/common/scalar.hpp>
#include <map>

namespace mk {

/// \brief `Settings` maps a key string to a Scalar value. This class is used
/// throughout MK to pass around settings. We generally pass Settings around by
/// value, so each function has its private copy.
///
/// Settings is a derived class of std::map<std::string, Scalar>. As such
/// it has all the methods of a standard std::map.
///
/// \since v0.1.0.
///
/// Support for mapping from string to any scalar type (rather than to
/// just strings) was added in MK v0.2.0.
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

    /// \brief `get()` returns the specified \p key if set; otherwise it
    /// returns the default value \p def_value.
    /// \throw ValueError if the value associated to \p key cannot be
    /// converted to the specified type.
    XX(Type, get, as)

    /// `as_noexcept` is like `as()` but return Error rather than throwing.
    XX(ErrorOr<Type>, get_noexcept, as_noexcept)

#undef XX

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
