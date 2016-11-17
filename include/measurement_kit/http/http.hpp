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

MK_DEFINE_ERR(MK_ERR_HTTP(0), UpgradeError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(1), GenericParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(2), UrlParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(3), MissingUrlSchemaError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(4), MissingUrlHostError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(5), MissingUrlError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(6), HttpRequestFailedError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(7), HeaderParserInternalError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(8), InvalidMaxRedirectsError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(9), InvalidRedirectUrlError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(10), EmptyLocationError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(11), TooManyRedirectsError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(12), ParsingHeadersInProgressError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(13), ParsingBodyInProgressError, "")

MK_DEFINE_ERR(MK_ERR_HTTP(14), ParserInvalidEofStateError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(15), ParserHeaderOverflowError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(16), ParserClosedConnectionError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(17), ParserInvalidVersionError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(18), ParserInvalidStatusError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(19), ParserInvalidMethodError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(20), ParserInvalidUrlError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(21), ParserInvalidHostError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(22), ParserInvalidPortError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(23), ParserInvalidPathError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(24), ParserInvalidQueryStringError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(25), ParserInvalidFragmentError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(26), ParserLfExpectedError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(27), ParserInvalidHeaderTokenError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(28), ParserInvalidContentLengthError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(29), ParserUnexpectedContentLengthError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(30), ParserInvalidChunkSizeError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(31), ParserInvalidConstantError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(32), ParserInvalidInternalStateError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(33), ParserStrictModeAssertionError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(34), ParserPausedError, "")

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
    unsigned short http_major;
    unsigned short http_minor;
    unsigned int status_code;
    std::string reason;
    Headers headers;
    std::string body;
};

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
