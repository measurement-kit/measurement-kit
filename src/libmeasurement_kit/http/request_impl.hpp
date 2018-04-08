// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_HTTP_REQUEST_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_HTTP_REQUEST_IMPL_HPP

#include <measurement_kit/common/json.hpp>
#include "src/libmeasurement_kit/common/mock.hpp"

#include "src/libmeasurement_kit/http/response_parser.hpp"
#include "src/libmeasurement_kit/http/http.hpp"
#include "src/libmeasurement_kit/net/connect.hpp"

namespace mk {
namespace http {

// TODO: mock more functions in request.cpp

template <MK_MOCK_AS(net::connect, net_connect)>
void request_connect_impl(Settings settings, Callback<Error, SharedPtr<Transport>> cb,
                          SharedPtr<Reactor> reactor = Reactor::global(),
                          SharedPtr<Logger> logger = Logger::global()) {
    if (settings.find("http/url") == settings.end()) {
        cb(MissingUrlError(), {});
        return;
    }
    ErrorOr<Url> url = parse_url_noexcept(settings.at("http/url"));
    if (!url) {
        cb(url.as_error(), {});
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
    net_connect(url->address, url->port, cb, settings, reactor, logger);
}

template <MK_MOCK_AS(mk::http::request, request)>
void request_json_string_impl(
      std::string method, std::string url, std::string data,
      http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    settings["http/url"] = url;
    settings["http/method"] = method;
    headers["Content-Type"] = "application/json";
    logger->debug("%s to %s (body: '%s')", method.c_str(), url.c_str(),
                  data.c_str());
    request(settings, headers, data,
            [=](Error error, SharedPtr<http::Response> response) {
                Json jresponse;
                if (error) {
                    cb(error, response, jresponse);
                    return;
                }
                error = json_process(response->body, [&](auto json) {
                    jresponse = std::move(json);
                });
                cb(error, response, jresponse);
            },
            reactor, logger, {}, 0);
}

} // namespace http
} // namespace mk
#endif
