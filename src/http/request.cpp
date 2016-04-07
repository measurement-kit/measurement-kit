// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/http/request.hpp"

namespace mk {
namespace http {

using namespace mk::net;

void request_connect(Settings settings, RequestConnectCb cb,
        Poller *poller, Logger *logger) {
    if (settings.find("url") == settings.end()) {
        cb(GenericError(), nullptr);
        return;
    }
    ErrorOr<Url> url = parse_url_noexcept(settings.at("url"));
    if (!url) {
        cb(url.as_error(), nullptr);
        return;
    }
    if (url->schema == "httpo") {
        // tor_socks_port takes precedence because it's more specific
        if (settings.find("tor_socks_port") != settings.end()) {
            std::string proxy = "127.0.0.1:";
            proxy += settings["tor_socks_port"];
            settings["socks5_proxy"] = proxy;
        } else if (settings.find("socks5_proxy") == settings.end()) {
            settings["socks5_proxy"] = "127.0.0.1:9050";
        }
        // XXX: socks5 not implemented yet
    }
    connect(url->address, url->port, cb, settings, logger, poller);
}

void request_send(Var<Transport> transport, Settings settings, Headers headers,
        std::string body, RequestSendCb callback) {
    RequestSerializer serializer;
    try {
        serializer = RequestSerializer(settings, headers, body);
    } catch (std::exception &) {
        callback(GenericError());
        return;
    }
    transport->on_error([transport, callback](Error error) {
        transport->on_error(nullptr);
        transport->on_flush(nullptr);
        callback(error);
    });
    transport->on_flush([transport, callback]() {
        transport->on_error(nullptr);
        transport->on_flush(nullptr);
        callback(NoError());
    });
    Buffer buff;
    serializer.serialize(buff);
    transport->write(buff);
}

} // namespace http
} // namespace mk
