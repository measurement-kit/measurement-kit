// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_HTTP_HTTP_HPP
#define MEASUREMENT_KIT_HTTP_HTTP_HPP

// Documentation: doc/api/http.md

#include <measurement_kit/net.hpp>

namespace mk {
namespace http {

/*
 _____
| ____|_ __ _ __ ___  _ __ ___
|  _| | '__| '__/ _ \| '__/ __|
| |___| |  | | | (_) | |  \__ \
|_____|_|  |_|  \___/|_|  |___/

    Error codes in the HTTP module.
*/

MK_DEFINE_ERR(MK_ERR_HTTP(0), UpgradeError, "http_upgrade_error")
MK_DEFINE_ERR(MK_ERR_HTTP(1), ParserError, "http_parser_error")
MK_DEFINE_ERR(MK_ERR_HTTP(2), UrlParserError, "http_url_parser_error")
MK_DEFINE_ERR(MK_ERR_HTTP(3), MissingUrlSchemaError, "http_missing_url_schema")
MK_DEFINE_ERR(MK_ERR_HTTP(4), MissingUrlHostError, "http_missing_url_host")
MK_DEFINE_ERR(MK_ERR_HTTP(5), MissingUrlError, "http_missing_url")
MK_DEFINE_ERR(MK_ERR_HTTP(6), HttpRequestFailedError, "http_request_failed")
MK_DEFINE_ERR(MK_ERR_HTTP(7), HeaderParserInternalError, "http_parser_internal_error")
MK_DEFINE_ERR(MK_ERR_HTTP(8), InvalidMaxRedirectsError, "http_invalid_max_redirects_setting")
MK_DEFINE_ERR(MK_ERR_HTTP(9), InvalidRedirectUrlError, "http_invalid_redirect_url")
MK_DEFINE_ERR(MK_ERR_HTTP(10), EmptyLocationError, "http_empty_location_header")
MK_DEFINE_ERR(MK_ERR_HTTP(11), TooManyRedirectsError, "http_too_many_redirects")

/*
 _   _      _
| | | |_ __| |
| | | | '__| |
| |_| | |  | |
 \___/|_|  |_|

    Code to parse URLs.
*/

class Url {
  public:
    std::string schema;
    std::string address;
    int port = 80;
    std::string path;
    std::string query;
    std::string pathquery;

    std::string str();
};

Url parse_url(std::string url);
ErrorOr<Url> parse_url_noexcept(std::string url);

/*
                                _
 _ __ ___  __ _ _   _  ___  ___| |_
| '__/ _ \/ _` | | | |/ _ \/ __| __|
| | |  __/ (_| | |_| |  __/\__ \ |_
|_|  \___|\__, |\__,_|\___||___/\__|
             |_|

    HTTP request and response structs, logic to make requests.
*/

class HeadersComparator {
  public:
    bool operator() (const std::string &l, const std::string &r) const;
};

using Headers = std::map<std::string, std::string, HeadersComparator>;

class Request {
  public:
    std::string method;
    Url url;
    std::string url_path;  // Allows to override `url.path` via Settings
    std::string protocol;
    Headers headers;
    std::string body;

    Request() {}
    Error init(Settings, Headers, std::string);
    void serialize(net::Buffer &, Var<Logger> logger = Logger::global());

    static ErrorOr<Var<Request>> make(Settings, Headers, std::string);
};

struct Response {
    Var<Request> request;
    Var<Response> previous;
    std::string response_line;
    unsigned short http_major = 0; // Initialize to know value
    unsigned short http_minor = 0; // Initialize to know value
    unsigned int status_code = 0;  // Initialize to know value
    std::string reason;
    Headers headers;
    std::string body;
};

ErrorOr<Url> redirect(const Url &orig_url, const std::string &location);

void request_connect(Settings, Callback<Error, Var<net::Transport>>,
                     Var<Reactor> = Reactor::global(),
                     Var<Logger> = Logger::global());

void request_send(Var<net::Transport>, Settings, Headers, std::string,
                  Callback<Error, Var<Request>>);

// Same as above except that the optional Request is passed in explicitly
void request_maybe_send(ErrorOr<Var<Request>>, Var<net::Transport>,
                        Callback<Error, Var<Request>>);

void request_recv_response(Var<net::Transport>, Callback<Error, Var<Response>>,
                           Var<Reactor> = Reactor::global(),
                           Var<Logger> = Logger::global());

void request_sendrecv(Var<net::Transport>, Settings, Headers, std::string,
                      Callback<Error, Var<Response>>,
                      Var<Reactor> = Reactor::global(),
                      Var<Logger> = Logger::global());

// Same as above except that the optional Request is passed in explicitly
void request_maybe_sendrecv(ErrorOr<Var<Request>>, Var<net::Transport>,
                            Callback<Error, Var<Response>>,
                            Var<Reactor> = Reactor::global(),
                            Var<Logger> = Logger::global());

/*
 * For settings the following options are defined:
 *
 *     {
 *       {"http/max_redirects", integer (default is zero)},
 *       {"http/url", std::string},
 *       {"http/ignore_body", boolean},
 *       {"http/method", "GET|DELETE|PUT|POST|HEAD|..."},
 *       {"http/http_version", "HTTP/1.1"},
 *       {"http/path", by default is taken from the url}
 *     }
 */

void request(Settings, Headers, std::string, Callback<Error, Var<Response>>,
             Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global(),
             Var<Response> previous = nullptr, int nredirects = 0);

inline void get(std::string url, Callback<Error, Var<Response>> cb,
                Headers headers = {}, Settings settings = {},
                Var<Reactor> reactor = Reactor::global(),
                Var<Logger> lp = Logger::global(),
                Var<Response> previous = nullptr,
                int nredirects = 0) {
    settings["http/method"] = "GET";
    settings["http/url"] = url;
    request(settings, headers, "", cb, reactor, lp, previous, nredirects);
}

} // namespace http
} // namespace mk
#endif
