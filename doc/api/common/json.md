# NAME
json &mdash; Code for parsing and processing JSON

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/common.hpp>

namespace mk {

template <typename Callable>
Error json_process(nlohmann::json &json, Callable &&fun);

template <typename Callable>
Error json_process_and_filter_errors(nlohmann::json &json, Callable &&fun);

template <typename Stringlike, typename Callable>
Error json_parse_and_process(Stringlike str, Callable &&fun);

template <typename Stringlike, typename Callable>
Error json_parse_process_and_filter_errors(Stringlike str, Callable &&fun);

}
```

# STABILITY

2 - Stable

# DESCRIPTION

The `json_process` template function takes as first argument a json
object and as second argument a callable that will manipulate such
json object. It applies the callable to the json, passing the json
to the callable as its first an unique argument. In doing that,
this template function will also filter out and return exceptions
that could be caused by processing a json in which some expected
fields are missing. Namely:

- `JsonKeyError`: will be returned when you assume that a specific
  json or sub-json is a dictionary and has a key, but such key is
  instead missing;

- `JsonDomainError`: will be returned when you assume that a specific
  json or sub-json is of a specific type, and hence you attempt to use
  it as such, but instead it is of another type.

The `json_process_and_filter_errors` template function additionally filters for
any other `Error` that may occur while evaluating `fun`.

The `json_parse_and_process` is similar to `json_process`, except that
it takes as first argument a string to be parsed into a json rather than
a json. This template function, accordingly, will first attempt to parse
the json, and then will call `json_process`. In addition to the above
listed exceptions, this template function can also fail with:

- `JsonParseError`: when the input string is not a valid json.

The `json_parse_process_and_filter_errors` template function additionally
filters for any `Error` that may occur while evaluating `fun`.

These template functions where introduced to enforce correct processing
of json and correct mixing of asynchronous and synchronous code without
having to repeat ourself too much. The following is an example of
*wrong* code that we would like to avoid in MK:

```C++
    // WRONG! DON'T DO THIS!
    try {
        auto json = nlohmann::json::parse(s);
        auto sub_json = json["some_key"];
        sub_json["k"] = 3.14;
        cb(NoError(), sub_json);
        return;
    } catch (const std::invalid_argument &) {
        cb(JsonParseError(), {});
        return;
    } catch (const std::out_of_range &) {
        cb(JsonKeyError(), {});
        return;
    } catch (const std::domain_error &) {
        cb(JsonDomainError(), {});
        return;
    }
```

The problem with the above code is that, in the non-exceptional case,
`cb` is called from within the `try` clause. Meaning that, in the event
that the callback is going to throw exceptions such as the ones that
are filtered above, they will be catched here, and the callback would
then be called *again*, the second time with an error value as its
first parameter.

To attempt to avoid this scenario, we have implemented the building
blocks described in this manual page, such that one could decouple
more easily the task of parsing and processing and the task of passing
the result of such parsing and processing to a callback. The above
code, refactored more correctly and using `json_parse_and_process` would
look like this:

```C++
    // Correct because the parse and process stage is clearly separated
    // from the stage where we forward results to a specific callback
    auto sub_json = nlohmann::json{};
    auto error = json_parse_and_process(s, [&](auto json) {
        sub_json = json["some_key"];
        sub_json["k"] = 3.14;
    });
    cb(error, sub_json);
```

# HISTORY

The `common/json` module appeared in MeasurementKit 0.7.0.
