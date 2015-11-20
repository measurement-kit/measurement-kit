# NAME
Headers -- HTTP headers.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

using namespace measurement_kit;

http::Headers headers;
```

# DESCRIPTION

The HTTP headers class represents HTTP headers. It is
actually a typedef for `std::map<std::string, std::string>`.

# HISTORY

The `Headers` class appeared in MeasurementKit 1.0.0.
