# NAME
parse_url &mdash; URL parsing

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

mk::http::Url mk::http::parse_url(std::string url);
mk::ErrorOr<mk::http::Url> mk::http::parse_url_noexcept(std::string url);
```

# STABILITY

2 - Stable

# DESCRIPTION

The `parse_url()` function parses the `url` argument into a `Url` structure:

```C++
class Url {
  public:
    std::string schema;
    std::string address;
    int port = 80;
    std::string path;
    std::string query;
    std::string pathquery;
};
```

On error one of the following exceptions could be thrown:

- `mk::http::UrlParserError`: error in URL parser
- `mk::http::MissingUrlSchemaError`: missing schema in parsed URL
- `mk::http::MissingUrlHostError`: missing host in parsed URL

The `parse_url_noexcept()` is equal to `parse_url()` except that rather
than throwing an exception of error it returns an `ErrorOr` wrapper.

# HISTORY

The `parse_url` module appeared in MeasurementKit 0.2.0.
