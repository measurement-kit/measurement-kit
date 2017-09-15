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

/// \brief `Json` is an alias for nlohmann::json. MK uses nlohmann::json for
/// all that concerns JSON processing. We use this alias such that we don't
/// have nlohmann::json explicitly visible in our headers.
///
/// This alias first appeared in measurement-kit v0.8.0.
using Json = nlohmann::json;

/// \brief The first form of `json_process` parses and processes a JSON.
/// \param data is the JSON to be parsed and then processed. \param fun is
/// the function that will process it.
///
/// json_process() will basically invoke the specified function and trap all
/// the possible exceptions caused by JSON processing, returning them as
/// errors. Additionally it will also trap all other Error or generic
/// exception triggered in the process and return them.
///
/// This function is meant to allow processing JSON in asynchronous context
/// knowing that no exception will be throw and that it is possible to report
/// the result of processing the JSON asynchronously to a callback.
///
/// \return NoError if parsing and processing were okay.
///
/// \return JsonParseError if it cannot parse the JSON.
///
/// \return JsonKeyError if the requested key is missing.
///
/// \return JsonDomainError if the JSON is of a specified type (e.g.
/// `null`) and you are treating it as another type (e.g. `array`).
///
/// \return any Error (or derived class) that you may throw.
///
/// \return JsonProcessingError if any non-Error exception is thrown.
///
/// \since v0.8.0.
Error json_process(const std::string &data, std::function<void(Json &)> &&fun);

/// \brief The second form of `json_process` is like the first form except
/// that the first argument is an already parsed JSON.
///
/// Of course, since it does not parse the JSON, this form of json_process
/// should not, in general, return JsonParseError.
///
/// \since v0.8.0.
Error json_process(Json &json, std::function<void(Json &)> &&fun);

} // namespace mk
#endif
