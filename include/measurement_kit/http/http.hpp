// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_HTTP_HTTP_HPP
#define MEASUREMENT_KIT_HTTP_HTTP_HPP

// Documentation: doc/api/http.md

#include <map>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <string>

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
MK_DEFINE_ERR(MK_ERR_HTTP(1), ParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(2), UrlParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(3), MissingUrlSchemaError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(4), MissingUrlHostError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(5), MissingUrlError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(6), HttpRequestFailedError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(7), HeaderParserInternalError, "")

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

using Headers = std::map<std::string, std::string>;

class Request {
  public:
    std::string method;
    Url url;
    std::string protocol;
    Headers headers;
    std::string path;
    std::string body;

    Request() {}
    Error init(Settings, Headers, std::string);
    void serialize(net::Buffer &);
};

struct Response {
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
                  Callback<Error>);

void request_recv_response(Var<net::Transport>, Callback<Error, Var<Response>>,
                           Var<Reactor> = Reactor::global(),
                           Var<Logger> = Logger::global());

void request_sendrecv(Var<net::Transport>, Settings, Headers, std::string,
                      Callback<Error, Var<Response>>,
                      Var<Reactor> = Reactor::global(),
                      Var<Logger> = Logger::global());

/*
 * For settings the following options are defined:
 *
 *     {
 *       {"http/follow_redirects", boolean},
 *       {"http/url", std::string},
 *       {"http/ignore_body", boolean},
 *       {"http/method", "GET|DELETE|PUT|POST|HEAD|..."},
 *       {"http/http_version", "HTTP/1.1"},
 *       {"http/path", by default is taken from the url}
 *     }
 *
 * Currently `http/follow_redirects` is not supported.
 */

void request(Settings, Headers, std::string, Callback<Error, Var<Response>>,
             Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

inline void get(std::string url, Callback<Error, Var<Response>> cb,
                Headers headers = {}, Settings settings = {},
                Var<Reactor> reactor = Reactor::global(),
                Var<Logger> lp = Logger::global()) {
    settings["http/method"] = "GET";
    settings["http/url"] = url;
    request(settings, headers, "", cb, reactor, lp);
}

} // namespace http
} // namespace mk
#endif
