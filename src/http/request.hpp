// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_HTTP_REQUEST_HPP
#define SRC_HTTP_REQUEST_HPP

#include <functional>
#include <map>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <measurement_kit/http.hpp>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include "src/http/request_serializer.hpp"
#include "src/http/response_parser.hpp"

namespace mk {
namespace http {

typedef std::function<void(Error)> RequestSendCb;
typedef Callback<Var<Response>> RequestRecvResponseCb;

template<void (*do_connect)(std::string, int,
        Callback<Var<Transport>>,
        Settings, Var<Logger>, Var<Reactor>) = net::connect>
void request_connect_impl(Settings settings, Callback<Var<Transport>> cb,
        Var<Reactor> reactor = Reactor::global(), Var<Logger> logger = Logger::global()) {
    if (settings.find("http/url") == settings.end()) {
        cb(MissingUrlError(), nullptr);
        return;
    }
    ErrorOr<Url> url = parse_url_noexcept(settings.at("http/url"));
    if (!url) {
        cb(url.as_error(), nullptr);
        return;
    }
    if (url->schema == "httpo") {
        // tor_socks_port takes precedence because it's more specific
        if (settings.find("net/tor_socks_port") != settings.end()) {
            std::string proxy = "127.0.0.1:";
            proxy += settings["net/tor_socks_port"];
            settings["net/socks5_proxy"] = proxy;
        } else if (settings.find("net/socks5_proxy") == settings.end()) {
            settings["net/socks5_proxy"] = "127.0.0.1:9050";
        }
    } else if (url->schema == "https") {
        settings["net/ssl"] = true;
    }
    do_connect(url->address, url->port, cb, settings, logger, reactor);
}

void request_send(Var<Transport>, Settings, Headers, std::string,
        RequestSendCb);

void request_recv_response(Var<Transport>, Callback<Var<Response>>,
        Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

void request_sendrecv(Var<Transport>, Settings, Headers, std::string,
        Callback<Var<Response>>, Var<Reactor> = Reactor::global(),
        Var<Logger> = Logger::global());

void request_cycle(Settings, Headers, std::string, Callback<Var<Response>>,
        Var<Reactor> = Reactor::global(), Var<Logger> = Logger::global());

} // namespace http
} // namespace mk
#endif
