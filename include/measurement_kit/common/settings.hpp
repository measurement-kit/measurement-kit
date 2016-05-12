// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SETTINGS_HPP
#define MEASUREMENT_KIT_COMMON_SETTINGS_HPP

#include <map>
#include <measurement_kit/common/settings_entry.hpp>
#include <string>

namespace mk {

class Settings : public std::map<std::string, SettingsEntry> {
  public:
    using std::map<std::string, SettingsEntry>::map;

    template <typename Type> Type get(std::string key, Type def_value) {
        if (find(key) == end()) {
            return def_value;
        }
        return at(key).as<Type>();
    }

    static Var<Settings> global() {
         static Var<Settings> singleton(new Settings);
         return singleton;
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
