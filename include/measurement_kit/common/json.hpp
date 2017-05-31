// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_JSON_HPP
#define MEASUREMENT_KIT_COMMON_JSON_HPP

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/sandbox.hpp>
#include <measurement_kit/ext/json.hpp>

namespace mk {

template <typename Callable>
Error json_process(nlohmann::json &json, Callable &&fun) {
    Error err = NoError();
    try {
        fun(json);
    } catch (const std::out_of_range &) {
        err = JsonKeyError();
    } catch (const std::domain_error &) {
        err = JsonDomainError();
    }
    return err;
}

template <typename Callable>
Error json_process_and_filter_errors(nlohmann::json &json, Callable &&fun) {
    return sandbox_for_errors([&]() {
        json_process(json, std::move(fun));
    });
}

template <typename Stringlike, typename Callable>
Error json_parse_and_process(Stringlike str, Callable &&fun) {
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(str);
    } catch (const std::invalid_argument &) {
        return JsonParseError();
    }
    return json_process(json, std::move(fun));
}

template <typename Stringlike, typename Callable>
Error json_parse_process_and_filter_errors(Stringlike str, Callable &&fun) {
    return sandbox_for_errors([&]() {
        return json_parse_and_process(str, std::move(fun));
    });
}

} // namespace mk
#endif
