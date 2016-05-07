// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_SETTINGS_HPP
#define MEASUREMENT_KIT_COMMON_SETTINGS_HPP

#include <map>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/error_or.hpp>
#include <sstream>
#include <string>

namespace mk {

/// This class is a specialization of the ordinary string that can also
/// store integral and boolean types. Once you have stored a value inside
/// of this class, you can access its string representation easily since
/// this class can be assigned to a string. If that fails, use the `str()`
/// method that explicitly converts to a string. To convert to integral
/// types, instead, use the `as<T>()` template method.
class SettingsEntry : public std::string {
  public:
    SettingsEntry() {}

    template <typename Type> SettingsEntry(Type value) {
        std::stringstream ss;
        ss << value;
        assign(ss.str());
    }

    template <typename Type> Type as() const {
        std::stringstream ss(c_str());
        Type value;
        ss >> value;
        if (!ss.eof()) {
            throw ValueError(); // Not all input was converted
        }
        if (ss.fail()) {
            throw ValueError(); // Input format was wrong
        }
        return value;
    }

    std::string str() const { return as<std::string>(); }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

class Settings : public std::map<std::string, SettingsEntry> {
  public:
    using std::map<std::string, SettingsEntry>::map;

    template <typename Type> Type get(std::string key, Type def_value) {
        if (find(key) == end()) {
            return def_value;
        }
        return at(key).as<Type>();
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

// Perhaps this could be moved in another place?
template <typename To, typename From> To lexical_cast(From f) {
    std::stringstream ss;
    To value;
    ss << f;
    ss >> value;
    if (!ss.eof()) {
        throw ValueError(); // Not all input was converted
    }
    if (ss.fail()) {
        throw ValueError(); // Input format was wrong
    }
    return value;
}

template <typename To, typename From>
ErrorOr<To> lexical_cast_noexcept(From f) {
    try {
        return lexical_cast<To, From>(f);
    } catch (Error err) {
        return err;
    }
}

} // namespace mk
#endif
