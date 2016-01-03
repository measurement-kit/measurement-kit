# NAME
http::XxxError -- HTTP errors.

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

http::UpgradeError();          // Received unexpected UPGRADE message
http::ParserError();           // Error in HTTP parser
http::UrlParserError();        // Error in URL parser
http::MissingUrlSchemaError(); // Missing schema in parsed URL
http::MissingUrlHostError();   // Missing host in parsed URL
```

# DESCRIPTION

Those are the errors reported by HTTP code.

# HISTORY

HTTP error classes appeared in MeasurementKit 0.1.0.
