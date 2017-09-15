// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/json.hpp>

namespace mk {

Error json_process(Json &json, std::function<void(Json &)> &&fun) {
    try {
        fun(json);
    } catch (const std::out_of_range &) {
        return JsonKeyError();
    } catch (const std::domain_error &) {
        return JsonDomainError();
    } catch (const Error &failure) {
        return failure;
    } catch (...) {
        return JsonProcessingError();
    }
    return NoError();
}

Error json_process(const std::string &data, std::function<void(Json &)> &&fun) {
    Json json;
    try {
        json = Json::parse(data);
    } catch (const std::invalid_argument &) {
        return JsonParseError();
    }
    return json_process(json, std::move(fun));
}

} // namespace mk
