// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SETTINGS_HPP
#define MEASUREMENT_KIT_COMMON_SETTINGS_HPP

#include <measurement_kit/common/scalar.hpp>
#include <map>

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
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
