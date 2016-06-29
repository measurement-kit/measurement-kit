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
                               mk::Callback<mk::Error, mk::Var<mk::net::Transport>>,
                               mk::Var<mk::Reactor> = mk::Reactor::global(),
                               mk::Var<mk::Logger> = mk::Logger::global());

void mk::http::request_send(mk::Var<mk::net::Transport> txp,
                            mk::Settings settings,
                            mk::http::Headers headers,
                            std::string body,
                            mk::Callback<mk::Error>);

void mk::http::request_recv_response(mk::Var<mk::net::Transport> txp,
                                     mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                                     mk::Var<mk::Reactor> = mk::Reactor::global(),
                                     mk::Var<Logger> = mk::Logger::global());

void mk::http::request_sendrecv(mk::Var<mk::net::Transport> txp,
                                mk::Settings settings,
                                mk::http::Headers headers,
                                std::string body,
                                mk::Callback<mk::Error, mk::Var<mk::http::Response>> callback,
                                mk::Var<mk::Reactor> = mk::Reactor::global(),
                                mk::Var<mk::Logger> = mk::Logger::global());
```

# STABILITY

2 - Stable

# DESCRIPTION

The `request()` function issues the asynchronous `HTTP` request specified by the
`settings` argument. The following settings are available:

- *http/follow_redirects*: if it set to yes, then the client will follow redirects
  (this is currently not implemented)

- *http/url*: the URL to use

- *http/ignore_body*: does not save response body (this is currently not implemented)

- *http/method*: the method to use (default: "GET")

- *http/http_version*: specify HTTP version (otherwise `HTTP/1.1 is used)

- *http/path*: path to use (if not specified the one inside the URL is used instead)

The `body` argument is either the request body or an empty string to send no request
body. The `callback` function is called when done; it receives the error that occurred
&mdash; or `NoError()` &mdash; as first argument and the `Response` as second argument.
Optionally you can also specify the `reactor` and the `logger` to be used.

The `get()` function is a wrapper for `request()` that sets for you `http/url` using
as input the `url` argument and `http/method` as `GET`. Unlike `request()` you cannot
set the body, because `GET` requests SHOULD NOT carry a body. All other arguments
have equal semantic.



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

The HTTP url object returned by `parse_url()` is as follows:

```
/// Represents a URL.
class Url {
  public:
    std::string schema;    /// URL schema
    std::string address;   /// URL address
    std::string port;      /// URL port
    std::string path;      /// URL path
    std::string query;     /// URL query
    std::string pathquery; /// URL path followed by optional query
};
```

# EXAMPLE
```C++
#include <measurement_kit/http.hpp>

// Parsing of URL (throws exception on error)
http::Url url = http::parse_url("http://www.google.com/");

// Same as above but using a maybe
Maybe<http::Url> url = http::parse_url_noexcept("http://www.kernel.org/");

// For sending a simple GET request you could use
mk::http::get("http://nexa.polito.it/",
    [](mk::Error err, mk::http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    });

// For sending a simple request request you could use
mk::http::request("HEAD", "http://nexa.polito.it/",
    [](mk::Error err, mk::http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    });

// Minimal invocation of request()
mk::http::request({
        "http/url" : "http://nexa.polito.it/",
    }, [](mk::Error err, mk::http::Response resp) {
        if (err) {
            throw err;
        }
        // TODO: Process the response
    });

// Invocation of request() with all optional arguments specified
mk::http::request({
        "http/follow_redirects" : "yes",       // default is no
        "http/url" : "http://nexa.polito.it/", // must be specified
        "http/method" : "PUT",                 // default is GET
        "http/path" : "/robots.txt",           // default is to use URL
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

# HISTORY

The `request` module appeared in MeasurementKit 0.2.0.
