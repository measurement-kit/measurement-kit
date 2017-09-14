// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_JSON_HPP
#define MEASUREMENT_KIT_COMMON_JSON_HPP

#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/nlohmann/json.hpp>
#include <string>

namespace mk {

using Json = nlohmann::json;

Error json_process(Json &json, std::function<void(Json &)> &&fun);

Error json_process(const std::string &data, std::function<void(Json &)> &&fun);

} // namespace mk
#endif
