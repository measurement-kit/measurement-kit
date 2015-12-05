# NAME
Headers -- HTTP headers.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

mk::http::Headers headers;
```

# DESCRIPTION

The HTTP headers class represents HTTP headers. It is
actually a typedef for `std::map<std::string, std::string>`.

# HISTORY

The `Headers` class appeared in MeasurementKit 0.1.0.
