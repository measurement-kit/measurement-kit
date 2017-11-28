// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
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
MK_DEFINE_ERR(MK_ERR_HTTP(12), ParserInvalidEofStateError, "http_parser_invalid_eof_state")
MK_DEFINE_ERR(MK_ERR_HTTP(13), ParserHeaderOverflowError, "http_parser_header_overflow")
MK_DEFINE_ERR(MK_ERR_HTTP(14), ParserClosedConnectionError, "http_parser_connection_closed")
MK_DEFINE_ERR(MK_ERR_HTTP(15), ParserInvalidVersionError, "http_parser_invalid_version")
MK_DEFINE_ERR(MK_ERR_HTTP(16), ParserInvalidStatusError, "http_parser_invalid_status")
MK_DEFINE_ERR(MK_ERR_HTTP(17), ParserInvalidMethodError, "http_parser_invalid_method")
MK_DEFINE_ERR(MK_ERR_HTTP(18), ParserInvalidUrlError, "http_parser_invalid_url")
MK_DEFINE_ERR(MK_ERR_HTTP(19), ParserInvalidHostError, "http_parser_invalid_host")
MK_DEFINE_ERR(MK_ERR_HTTP(20), ParserInvalidPortError, "http_parser_invalid_port")
MK_DEFINE_ERR(MK_ERR_HTTP(21), ParserInvalidPathError, "http_parser_invalid_path")
MK_DEFINE_ERR(MK_ERR_HTTP(22), ParserInvalidQueryStringError, "http_parser_invalid_query_string")
MK_DEFINE_ERR(MK_ERR_HTTP(23), ParserInvalidFragmentError, "http_parser_invalid_fragment")
MK_DEFINE_ERR(MK_ERR_HTTP(24), ParserLfExpectedError, "http_parser_expected_lf")
MK_DEFINE_ERR(MK_ERR_HTTP(25), ParserInvalidHeaderTokenError, "http_parser_invalid_header_token")
MK_DEFINE_ERR(MK_ERR_HTTP(26), ParserInvalidContentLengthError, "http_parser_invalid_content_length")
MK_DEFINE_ERR(MK_ERR_HTTP(27), ParserUnexpectedContentLengthError, "http_parser_unexpected_content_length")
MK_DEFINE_ERR(MK_ERR_HTTP(28), ParserInvalidChunkSizeError, "http_parser_invalid_chunk_size")
MK_DEFINE_ERR(MK_ERR_HTTP(29), ParserInvalidConstantError, "http_parser_invalid_constant")
MK_DEFINE_ERR(MK_ERR_HTTP(30), ParserInvalidInternalStateError, "http_parser_invalid_internal_state")
MK_DEFINE_ERR(MK_ERR_HTTP(31), ParserStrictModeAssertionError, "http_parser_strict_mode_assertion")
MK_DEFINE_ERR(MK_ERR_HTTP(32), ParserPausedError, "http_parser_paused")
MK_DEFINE_ERR(MK_ERR_HTTP(33), GenericParserError, "http_parser_generic_error")

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
    void serialize(net::Buffer &, SharedPtr<Logger> logger = Logger::global());

    static ErrorOr<SharedPtr<Request>> make(Settings, Headers, std::string);
};

struct Response {
    SharedPtr<Request> request;
    SharedPtr<Response> previous;
    std::string response_line;
    unsigned short http_major = 0; // Initialize to know value
    unsigned short http_minor = 0; // Initialize to know value
    unsigned int status_code = 0;  // Initialize to know value
    std::string reason;
    Headers headers;
    std::string body;
};

ErrorOr<Url> redirect(const Url &orig_url, const std::string &location);

void request_connect(Settings, Callback<Error, SharedPtr<net::Transport>>,
                     SharedPtr<Reactor> = Reactor::global(),
                     SharedPtr<Logger> = Logger::global());

void request_send(SharedPtr<net::Transport>, Settings, Headers, std::string,
                  SharedPtr<Logger>, Callback<Error, SharedPtr<Request>>);

// Same as above except that the optional Request is passed in explicitly
void request_maybe_send(ErrorOr<SharedPtr<Request>>, SharedPtr<net::Transport>,
                        SharedPtr<Logger>, Callback<Error, SharedPtr<Request>>);

void request_recv_response(SharedPtr<net::Transport>, Callback<Error, SharedPtr<Response>>,
                           Settings = {}, SharedPtr<Reactor> = Reactor::global(),
                           SharedPtr<Logger> = Logger::global());

void request_sendrecv(SharedPtr<net::Transport>, Settings, Headers, std::string,
                      Callback<Error, SharedPtr<Response>>,
                      SharedPtr<Reactor> = Reactor::global(),
                      SharedPtr<Logger> = Logger::global());

// Same as above except that the optional Request is passed in explicitly
void request_maybe_sendrecv(ErrorOr<SharedPtr<Request>>, SharedPtr<net::Transport>,
                            Callback<Error, SharedPtr<Response>>,
                            Settings = {},
                            SharedPtr<Reactor> = Reactor::global(),
                            SharedPtr<Logger> = Logger::global());

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

void request(Settings, Headers, std::string, Callback<Error, SharedPtr<Response>>,
             SharedPtr<Reactor> = Reactor::global(), SharedPtr<Logger> = Logger::global(),
             SharedPtr<Response> previous = {}, int nredirects = 0);

inline void get(std::string url, Callback<Error, SharedPtr<Response>> cb,
                Headers headers = {}, Settings settings = {},
                SharedPtr<Reactor> reactor = Reactor::global(),
                SharedPtr<Logger> lp = Logger::global(),
                SharedPtr<Response> previous = {},
                int nredirects = 0) {
    settings["http/method"] = "GET";
    settings["http/url"] = url;
    request(settings, headers, "", cb, reactor, lp, previous, nredirects);
}

/*
   _
  (_)___  ___  _ __
  | / __|/ _ \| '_ \
  | \__ \ (_) | | | |
 _/ |___/\___/|_| |_|
|__/
*/

void request_json_string(
      std::string method, std::string url, std::string data,
      http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

void request_json_no_body(
      std::string method, std::string url, http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

void request_json_object(
      std::string method, std::string url, Json jdata,
      http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

} // namespace http
} // namespace mk
#endif
