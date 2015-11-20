# NAME
Response -- HTTP reponse.

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

using namespace measurement_kit;

http::Response response;

std::string s = response.response_line;
unsigned short mjr = response.http_major;
unsigned short mnr = response.http_minor;
unsigned int sc = response.status_code;
http::Headers hh = response.headers;
std::string b = response.body;
```

# DESCRIPTION

The HTTP response class contains the fields of an HTTP response
including response line, headers, and body.

# HISTORY

The `Response` class appeared in MeasurementKit 1.0.0.
