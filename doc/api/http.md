# NAME
mk::http -- MeasurementKit HTTP library

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/http.hpp>

// Minimal invocation of request()
mk::http::request({
        "url" : "http://nexa.polito.it/",
    }, [](mk::Error err, mk::http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    });

// Invocation of request() with all optional arguments specified
mk::http::request({
        "follow_redirects" : "yes",       // default is no
        "url" : "http://nexa.polito.it/", // must be specified
        "method" : "PUT",                 // default is GET
        "path" : "/robots.txt",           // default is to use URL
        "http_version" : "HTTP/1.0"       // default is HTTP/1.1
    }, [](mk::Error err, mk::http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    }, {
        {"Content-Type", "text/plain"},
        {"User-Agent", "Antani/1.0"}
    }, "THIS IS THE BODY");
```

# DESCRIPTION

The `http::request()` function allows you to issue HTTP requests and
to receive the corresponding responses. It receives the following arguments:

- *settings*: Settings that configure the http implementation to send a
  specific HTTP request; available settings are:

    - *follow_redirects*: if it set to yes, then the client
      will follow redirects (this is currently not implemented)

    - *url*: the URL to use

    - *method*: the method to use (default: "GET")

    - *path*: path to use (if not specified the one inside
       the URL is used instead)

    - *http_version*: specify HTTP version (otherwise
       `HTTP/1.1 is used)

- *callback*: Function called when either a response is received
  or an error occurs

- *headers*: Optional headers object that specifies the HTTP headers
  to be passed along with the request

- *body*: Optional body for the request

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
currently is alias for `std::map<std::string, std::string>`.

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
