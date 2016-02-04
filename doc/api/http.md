# NAME
mk::http -- MeasurementKit HTTP library

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

mk::http::Client client;

client.request(
    {
      "follow_redirects" : "yes",       // default is no
      "url" : "http://nexa.polito.it/", // must be specified
      "method" : "PUT",                 // default is GET
      "path" : "/robots.txt",           // default is to use URL
      "http_version" : "HTTP/1.0"       // default is HTTP/1.1
    },
    {{"Content-Type", "text/plain"}, {"User-Agent", "Antani/1.0"}},
    "THIS IS THE BODY", [](mk::Error err, mk::http::Response resp) {
        if (err) throw err;
        // Process the response
    });
```

# DESCRIPTION

The HTTP client class allows you to issue HTTP requests and
to receive the corresponding responses.

To this end, call the `request()` method of the client, which
receives the following arguments:

- *settings*: Settings that configure the client to send a
  specific HTTP request; available settings are:

    - `follow_redirects`: if it set to yes, then the client
      will follow redirects (this is currently not implemented)

    - `url`: the URL to use

    - `method`: the method to use (default: `GET`)

    - `path`: path to use (if not specified the one inside
       the URL is used instead)

    - `http_version`: specify HTTP version (otherwise
       `HTTP/1.1 is used)

- *headers*: Headers object that specifies the HTTP headers to
  be passed along with the request

- *body*: Body for the request (pass an empty string to
  send no body along with the request)

- *callback*: Function called when either a response is received
  or an error occurs

Beware that, in case of early error, the callback MAY be called
immediately by the `request()` method.

In case of success, the error argument of the callback is passed an
instance of `mk::NoError()`. Otherwise, the error that occurred is
reported. Among all the possible errors, the following are defined by
MeasurementKit HTTP implementation:

- `UpgradeError`: received unexpected UPGRADE message
- `ParserError`: error in HTTP parser
- `UrlParserError`: error in URL parser
- `MissingUrlSchemaError`: missing schema in parsed URL
- `MissingUrlHostError`: missing host in parsed URL

HTTP headers are represented by the `http::Headers` typedef that
is alias for `std::map<std::string, std::string>`.

The HTTP response object returned by the callback contains the
following fields:

```C++
struct Response {
    std::string response_line;
    unsigned short http_major;
    unsigned short http_minor;
    unsigned int status_code;
    std::string reason;
    Headers headers;
    std::string body;
};
```

# HISTORY

The `mk::http` library appeared in MeasurementKit 0.1.0.
