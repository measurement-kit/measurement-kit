# NAME

`measurement_kit/common/json.hpp`

# LIBRARY

measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)

# SYNOPSIS

```C++
#ifndef MEASUREMENT_KIT_COMMON_JSON_HPP
#define MEASUREMENT_KIT_COMMON_JSON_HPP

namespace mk {

using Json = nlohmann::json;

Error json_process(const std::string &data, std::function<void(Json &)> &&fun);

Error json_process(Json &json, std::function<void(Json &)> &&fun);

} // namespace mk
#endif
```

# DESCRIPTION

`Json` is an alias for nlohmann::json. MK uses nlohmann::json for all that concerns JSON processing. We use this alias such that we don't have nlohmann::json explicitly visible in our headers. 

This alias first appeared in measurement-kit v0.8.0.

The first form of `json_process` parses and processes a JSON. Parameter data is the JSON to be parsed and then processed. Parameter fun is the function that will process it. 

json_process() will basically invoke the specified function and trap all the possible exceptions caused by JSON processing, returning them as errors. Additionally it will also trap all other Error or generic exception triggered in the process and return them. 

This function is meant to allow processing JSON in asynchronous context knowing that no exception will be throw and that it is possible to report the result of processing the JSON asynchronously to a callback. 

The expected usage pattern is like: 

```C++ 

make_http_request(/* params */, [](Error err, http::Response response) { if (err || response->code != 200) { callback((err) ? err : GenericError()); return; } err = json_process(response->body, [](Json &root) { if (!root["status"] == "success") { throw AuthenticatorError("api_failure"); }); if (!root["authenticated") { throw AuthenticatorError("not_authenticated"); } }); callback(err); }); 

Returns NoError if parsing and processing were okay. 

Returns JsonParseError if it cannot parse the JSON. 

Returns JsonKeyError if the requested key is missing. 

Returns JsonDomainError if the JSON is of a specified type (e.g. `null`) and you are treating it as another type (e.g. `array`). 

Returns any Error (or derived class) that you may throw. 

Returns JsonProcessingError if any non-Error exception is thrown. 

Available since measurement-kit v0.8.0.

The second form of `json_process` is like the first form except that the first argument is an already parsed JSON. 

Of course, since it does not parse the JSON, this form of json_process should not, in general, return JsonParseError. 

Available since measurement-kit v0.8.0.

