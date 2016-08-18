// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_HTTP_REQUEST_HPP
#define SRC_HTTP_REQUEST_HPP

#include "src/http/response_parser.hpp"
#include <measurement_kit/http.hpp>

namespace mk {
namespace http {

// TODO: mock more functions in request.cpp

template <MK_MOCK_NAMESPACE(net, connect)>
void request_connect_impl(Settings settings, Callback<Error, Var<Transport>> cb,
                          Var<Reactor> reactor = Reactor::global(),
                          Var<Logger> logger = Logger::global()) {
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
            // XXX The following is a violation of layers because we are
            // setting a variable that NET code should set for itself; we
            // should do nothing in this case, lower layer should do
            std::string proxy = "127.0.0.1:";
            proxy += settings["net/tor_socks_port"];
            settings["net/socks5_proxy"] = proxy;
        } else if (settings.find("net/socks5_proxy") == settings.end()) {
            settings["net/socks5_proxy"] = "127.0.0.1:9050";
        }
    } else if (url->schema == "https") {
        settings["net/ssl"] = true;
    }
    net_connect(url->address, url->port, cb, settings, logger, reactor);
}

} // namespace http
} // namespace mk
#endif
