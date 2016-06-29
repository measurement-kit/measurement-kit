# NAME
request &mdash; issue HTTP requests

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSYS

```C++
#include <measurement_kit/http.hpp>

void mk::http::request(mk::Settings settings,
                       mk::http::Headers headers,
                       std::string body,
                       mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                       mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                       mk::Var<mk::Logger> = mk::Logger::global());

void mk::http::get(std::string url,
                   mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                   mk::http::Headers headers = {},
                   mk::Settings settings = {},
                   mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                   mk::Var<mk::Logger> = mk::Logger::global());

void mk::http::request_connect(mk::Settings settings,
                               mk::Callback<mk::Error, mk::Var<mk::net::Transport>> callback,
                               mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                               mk::Var<mk::Logger> logger = mk::Logger::global());

void mk::http::request_send(mk::Var<mk::net::Transport> txp,
                            mk::Settings settings,
                            mk::http::Headers headers,
                            std::string body,
                            mk::Callback<mk::Error> callback);

void mk::http::request_recv_response(mk::Var<mk::net::Transport> txp,
                                     mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                                     mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                                     mk::Var<Logger> logger = mk::Logger::global());

void mk::http::request_sendrecv(mk::Var<mk::net::Transport> txp,
                                mk::Settings settings,
                                mk::http::Headers headers,
                                std::string body,
                                mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                                mk::Var<mk::Reactor> reactor = mk::Reactor::global(),
                                mk::Var<mk::Logger> logger = mk::Logger::global());
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `request()` function issues the asynchronous `HTTP` request specified by the
`settings` argument. The following settings are available:

- *http/follow_redirects*: if it set to yes, then the client will follow redirects
  (this is currently not implemented)

- *http/url*: the URL to use

- *http/ignore_body*: does not save response body (this is currently not implemented)

- *http/method*: the method to use (default: "GET")

- *http/http_version*: specify HTTP version (otherwise `HTTP/1.1` is used)

- *http/path*: path to use (if not specified the one inside the URL is used instead)

The `body` argument is either the request body or an empty string to send no request
body. The `callback` function is called when done; it receives the error that occurred
&mdash; or `NoError()` &mdash; as first argument and the `Response` as second argument.
Optionally you can also specify the `reactor` and the `logger` to be used.

The `get()` function is a wrapper for `request()` that sets for you `http/url` using
as input the `url` argument and `http/method` as `GET`. Unlike `request()` you cannot
set the body, because `GET` requests SHOULD NOT carry a body. All other arguments
have equal semantic.

Both `request()` and `get()` support `SSL` if the URL schema is `https` and SOCKS5
proxying as described below for `request_connect()`. Also, both
functions close the established connection when the response is received. To implement
keep-alive semantic, use the following, lower-level functions.

The `request_connect()` function establishes a TCP (and possibly SSL) connection
towards an HTTP (or HTTPS) server. It uses as input the specified `settings`
and, when done, it invokes `callback` with the error that occurred &mdash; or
`NoError()` &mdash; as the first argument and the connected transport wrapped by
a `Var` as the second argument. In case of error, the transport SHOULD be `nullptr`.
You can also specify an optional `reactor` and `logger`. The `settings` that
matter to this function are the following:

- *http/url*: use to find out the address to connect to and whether to connect
  using TCP (if schema is `http`) or SSL (if schema is `https`). Additionally, if
  schema is `httpo` (for `HTTP onion`), this function MAY set SOCKS5 proxy settings
  ot use locally running instance of `tor`.

- *net/tor_socks_port*: if this setting is present, this function would pass
  `127.0.0.1:${net/tor_socks_port}` as SOCKS5 proxy to `net/connect()`.

- *net/socks5_proxy*: if this setting is present and *net/tor_socks_port* is
  not present, then this setting is passed verbatim to `net/connect()`.

- all other settings that matter to `net/connect()`.

The `request_send()` function send an HTTP request asynchronously over the `txp`
transport, using `settings`, `headers`, and `body` to construct the request,
and calls `callback` when done. The settings that matter to this function are
`http/url`, `http/http_version`, `http/method`, and `http/path` &mdash; all
already described above. The callback receives the error that occurred or
`NoError()` in case of success.

The `request_recv_response()` function receives an HTTP response asynchronously
using the `txp` transport and calling `callback` when done. You can optionally
specify a `reactor` and a `logger` to use. On error, the callback receives it as
its first argument; otherwise, the first argument is `NoError()` and the second
argument is the received HTTP response wrapped by a `Var`.

The `request_sendrecv()` function combines the `request_send()` and the
`request_recv_response()` functions into a single call.

Beware that, in case of early error, the callback MAY be called immediately
by any of the above functions. The following errors may occurr:

- `UpgradeError`: received unexpected UPGRADE message
- `ParserError`: error in HTTP parser
- `UrlParserError`: error in URL parser
- `MissingUrlSchemaError`: missing schema in parsed URL
- `MissingUrlHostError`: missing host in parsed URL
- `MissingUrlError`: no URL was passed to a function that required it
- `HttpRequestFailedError`: the response status code indicates an HTTP error (e.g. `404`)
- `HeaderParserInternalError`: the response headers parser encountered an error

HTTP headers are represented by the `http::Headers` typedef that
currently is alias for `std::map<std::string, std::string>`.

The HTTP response object returned by several callbacks is like:

```C++
class Response {
  public:
    std::string response_line;
    unsigned short http_major;
    unsigned short http_minor;
    unsigned int status_code;
    std::string reason;
    Headers headers;
    std::string body;
};
```

# EXAMPLE
```C++
#include <measurement_kit/http.hpp>

using namespace mk;

http::request({
        {"http/follow_redirects", "yes"},       // default is no
        {"http/url", "http://nexa.polito.it/"}, // must be specified
        {"http/method", "PUT"},                 // default is GET
        {"http/path", "/robots.txt"},           // default is to use URL's path
        {"http/http_version", "HTTP/1.0"},      // default is HTTP/1.1
    }, [](Error err, http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    }, {
        {"Content-Type", "text/plain"},
        {"User-Agent", "Antani/1.0"}
    }, "THIS IS THE BODY");
```

# BUGS

- It is not possible to search HTTP headers in a case insensitive fashion

- The `Response::response_line` field is always empty

- The `http/ignore_body` setting is not implemented

- The `http/follow_redirects` setting is not implemented

# HISTORY

The `request` module appeared in MeasurementKit 0.2.0.
