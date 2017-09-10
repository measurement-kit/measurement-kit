// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_SETTINGS_GET_HPP
#define PRIVATE_COMMON_SETTINGS_GET_HPP

#include <measurement_kit/common/error_or.hpp>
#include <measurement_kit/common/lexical_cast.hpp>
#include <measurement_kit/common/settings.hpp>
#include <map>
#include <string>

namespace mk {

template <typename Type>
Type settings_get(const Settings &settings, std::string key, Type value) {
    return settings.count(key) ? lexical_cast<Type>(settings.at(key)) : value;
}

template <typename Type>
ErrorOr<Type> settings_get_noexcept(const Settings &settings, std::string key,
                                    Type value) {
    return settings.count(key) ? lexical_cast_noexcept<Type>(settings.at(key))
                               : ErrorOr<Type>{value};
}

} // namespace mk
#endif
