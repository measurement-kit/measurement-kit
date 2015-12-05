# NAME
Client -- HTTP client.

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

# HISTORY

The `Response` class appeared in MeasurementKit 0.1.0.
