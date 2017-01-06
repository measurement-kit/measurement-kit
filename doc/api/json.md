# NAME
json &mdash; module containing code used to handle json

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/json.hpp>

template <typename T> ErrorOr<T> json_noexcept(std::function<T()> fn);
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `json` module contains `nhlohmann::json` as external
module and a wrapper function to use the feature of it
without throwing exceptions.

The `json_noexcept` function will catch every exception
throwed inside of the lambda returning it inside the
`ErrorOr<>` return value. An example of usage is:

```C++
ErrorOr<json> result = json_noexcept<json>([]() {
		auto value = json::parse("{\"key\":\"value\"}");
		return value["key"];
		});
if (!result) {
	/* handle the error */
}
/* handle the value stored in result */
```

# HISTORY

The `json` module appeared in MeasurementKit 0.4.0.
