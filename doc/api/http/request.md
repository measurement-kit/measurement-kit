# NAME
request &mdash; issue HTTP requests

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSYS

```C++
#include <measurement_kit/http.hpp>

void request(Settings settings,
             Headers headers,
             std::string body,
             Callback<Error, Var<Response>> callback,
             Var<Reactor> reactor = Reactor::global(),
             Var<Logger> = Logger::global());

void get(std::string url,
         Callback<Error, Var<Response>> callback,
         Headers headers = {},
         Settings settings = {},
         Var<Reactor> reactor = Reactor::global(),
         Var<Logger> logger = Logger::global());

void request_json_string(
    std::string method, std::string url, std::string data,
    http::Headers headers,
    Callback<Error, Var<http::Response>, nlohmann::json> cb, Settings settings,
    Var<Reactor> reactor, Var<Logger> logger);

void request_json_no_body(
    std::string method, std::string url, http::Headers headers,
    Callback<Error, Var<http::Response>, nlohmann::json> cb, Settings settings,
    Var<Reactor> reactor, Var<Logger> logger);

void request_json_object(
    std::string method, std::string url, nlohmann::json jdata,
    http::Headers headers,
    Callback<Error, Var<http::Response>, nlohmann::json> cb, Settings settings,
    Var<Reactor> reactor, Var<Logger> logger);

void request_connect(Settings settings,
                     Callback<Error, Var<net::Transport>> callback,
                     Var<Reactor> reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global());

void request_send(Var<net::Transport> txp,
                  Settings settings,
                  Headers headers,
                  std::string body,
                  Callback<Error> callback);

void request_recv_response(Var<net::Transport> txp,
                           Callback<Error, Var<Response>> callback,
                           Var<Reactor> reactor = Reactor::global(),
                           Var<Logger> logger = Logger::global());

void request_sendrecv(Var<net::Transport> txp,
                      Settings settings,
                      Headers headers,
                      std::string body,
                      Callback<Error, Var<Response>> callback,
                      Var<Reactor> reactor = Reactor::global(),
                      Var<Logger> logger = Logger::global());

ErrorOr<Url> redirect(const Url &orig_url, const std::string &location);
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `request()` function issues the asynchronous `HTTP` request
specified by the `settings` argument. The following settings are
available:

- *http/max_redirects*: maximum number of redirects to follow, defaults to zero

- *http/url*: the URL to use

- *http/ignore_body*: does not save response body (this is currently
  not implemented)

- *http/method*: the method to use (default: "GET")

- *http/http_version*: specify HTTP version (otherwise `HTTP/1.1` is used)

- *http/path*: path to use (if not specified the one inside the URL
  is used instead)

The `body` argument is either the request body or an empty string
to send no request body. The `callback` function is called when
done; it receives the error that occurred &mdash; or `NoError()`
&mdash; as first argument and the `Response` as second argument.
Optionally you can also specify the `reactor` and the `logger` to
be used.

The `get()` function is a wrapper for `request()` that sets for you
`http/url` using as input the `url` argument and `http/method` as
`GET`. Unlike `request()` you cannot set the body, because `GET`
requests SHOULD NOT carry a body. All other arguments have equal
semantic.

The `request_json_string()` function is a wrapper for `request()`
that sends a JSON request (serialized as a string as the `data`
argument) and gives back a JSON response (as a `nlohmann::json`).

The `request_json_no_body()` function is a wrapper for `request()`
that expects the response to be a JSON.

The `request_json_object()` function is a wrapper for `request()`
that sends a JSON request (represented by a `nlohmann::json`)
and gives back a JSON response (again represented by a `nlohmann::json`).

Both `request()` and `get()` support `SSL` if the URL schema is
`https` and SOCKS5 proxying as described below for `request_connect()`.
Also, both functions close the established connection when the
response is received. To implement keep-alive semantic, use the
following, lower-level functions:

The `request_connect()` function establishes a TCP (and possibly
SSL) connection towards an HTTP (or HTTPS) server. It uses as input
the specified `settings` and, when done, it invokes `callback` with
the error that occurred &mdash; or `NoError()` &mdash; as the first
argument and the connected transport wrapped by a `Var` as the
second argument. In case of error, the transport SHOULD be `nullptr`.
You can also specify an optional `reactor` and `logger`. The
`settings` that matter to this function are the following:

- *http/url*: used to find out the address to connect to and whether
  to connect using TCP (if schema is `http`) or SSL (if schema is
  `https`).  Additionally, if schema is `httpo` (for `HTTP onion`),
  this function MAY set SOCKS5 proxy settings ot use locally running
  instance of `tor`.

- *net/tor_socks_port*: if this setting is present, this function would pass
  `127.0.0.1:${net/tor_socks_port}` as SOCKS5 proxy to `net/connect()`.

- *net/socks5_proxy*: if this setting is present and *net/tor_socks_port* is
  not present, then this setting is passed verbatim to `net/connect()`.

- all other settings that matter to `net/connect()`.

The `request_send()` function send an HTTP request asynchronously
over the `txp` transport, using `settings`, `headers`, and `body`
to construct the request, and calls `callback` when done. The
settings that matter to this function are `http/url`, `http/http_version`,
`http/method`, and `http/path` &mdash; all already described above.
The callback receives the error that occurred or `NoError()` in
case of success.

The `request_recv_response()` function receives an HTTP response
asynchronously using the `txp` transport and calling `callback`
when done. You can optionally specify a `reactor` and a `logger`
to use. On error, the callback receives it as its first argument;
otherwise, the first argument is `NoError()` and the second argument
is the received HTTP response wrapped by a `Var`.

The `request_sendrecv()` function combines the `request_send()` and
the `request_recv_response()` functions into a single call.

Beware that, in case of early error, the callback MAY be called
immediately by any of the above functions. The following errors may
occurr:

- `UpgradeError`: received unexpected UPGRADE message

- `ParserError`: error in HTTP parser

- `UrlParserError`: error in URL parser

- `MissingUrlSchemaError`: missing schema in parsed URL

- `MissingUrlHostError`: missing host in parsed URL

- `MissingUrlError`: no URL was passed to a function that required it

- `HeaderParserInternalError`: the response headers parser encountered an error

HTTP headers are represented by the `http::Headers` typedef that
currently is alias for `std::map<std::string, std::string>` where
the comparison of header keys is case insensitive.

The HTTP response object returned by several callbacks is like:

```C++
class Response {
  public:
    std::string response_line;
    unsigned short http_major = 0;
    unsigned short http_minor = 0;
    unsigned int status_code = 0;
    std::string reason;
    Headers headers;
    std::string body;
};
```

The `redirect()` function will construct a new URL from the existing
URL and a location header, basically implementing MK redirection
logic.

# EXAMPLE

See `example/http/request.cpp`.

# BUGS

- The `http/ignore_body` setting is not implemented.

- The `Var<Response>` returned by the various callbacks MAY be pointing
  to `nullptr` and, moreover, there MAY be cases where `Var<Response> response`
  is pointing to a valid response but `response->request` is `nullptr`.

# HISTORY

The `request` module appeared in MeasurementKit 0.2.0. Support for calling
JSON APIs was added in MK v0.7.0.
