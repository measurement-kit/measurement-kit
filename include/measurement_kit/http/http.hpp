// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_HTTP_HTTP_HPP
#define MEASUREMENT_KIT_HTTP_HTTP_HPP

#include <map>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <string>

namespace mk {
namespace http {

MK_DEFINE_ERR(MK_ERR_HTTP(0), UpgradeError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(1), ParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(2), UrlParserError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(3), MissingUrlSchemaError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(4), MissingUrlHostError, "")
MK_DEFINE_ERR(MK_ERR_HTTP(5), MissingUrlError, "")

using Headers = std::map<std::string, std::string>;

struct Response {
    std::string response_line;
    unsigned short http_major;
    unsigned short http_minor;
    unsigned int status_code;
    std::string reason;
    Headers headers;
    std::string body;
};

void request(Settings settings, Callback<Error, Response> cb, Headers headers = {},
             std::string body = "", Var<Logger> lp = Logger::global(),
             Var<Reactor> reactor = Reactor::global());

inline void request(Settings settings, Headers headers, std::string body,
        Callback<Error, Response> cb, Var<Logger> lp = Logger::global(),
        Var<Reactor> reactor = Reactor::global()) {
    request(settings, cb, headers, body, lp, reactor);
}

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

inline void get(std::string url, Callback<Error, Response> cb,
                Headers headers = {}, std::string body = "",
                Settings settings = {}, Var<Logger> lp = Logger::global(),
                Var<Reactor> reactor = Reactor::global()) {
    settings["http/method"] = "GET";
    settings["http/url"] = url;
    request(settings, cb, headers, body, lp, reactor);
}

inline void request(std::string method, std::string url, Callback<Error, Response> cb,
                    Headers headers = {}, std::string body = "",
                    Settings settings = {}, Var<Logger> lp = Logger::global(),
                    Var<Reactor> reactor = Reactor::global()) {
    settings["http/method"] = method;
    settings["http/url"] = url;
    request(settings, cb, headers, body, lp, reactor);
}

void request_connect(Settings, Callback<Error, Var<net::Transport>>,
                     Var<Reactor> = Reactor::global(),
                     Var<Logger> = Logger::global());

void request_send(Var<net::Transport>, Settings, Headers, std::string,
        Callback<Error>);

void request_recv_response(Var<net::Transport>, Callback<Error, Var<Response>>,
        Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

void request_sendrecv(Var<net::Transport>, Settings, Headers, std::string,
        Callback<Error, Var<Response>>, Var<Reactor> = Reactor::global(),
        Var<Logger> = Logger::global());

void request_cycle(Settings, Headers, std::string, Callback<Error, Var<Response>>,
        Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

} // namespace http
} // namespace mk
#endif
