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

void request_recv_response(Var<Transport> transport, RequestRecvResponseCb cb,
        Poller *poller, Logger *logger) {
    Var<ResponseParserNg> parser(new ResponseParserNg);
    Var<Response> response(new Response);
    Var<bool> prevent_emit(new bool(false));

    transport->on_data([=](Buffer data) { parser->feed(data); });
    parser->on_response([=](Response r) { *response = r; });
    parser->on_body([=](std::string s) { response->body += s; });

    parser->on_end([=]() {
        if (*prevent_emit == true) {
            return;
        }
        transport->emit_error(NoError());
    });
    transport->on_error([=](Error err) {
        if (err == EofError()) {
            // Calling parser->on_eof() could trigger parser->on_end() and
            // we don't want this function to call ->emit_error()
            *prevent_emit = true;
            parser->eof();
        }
        cb(err, response);
        transport->on_error(nullptr);
        transport->on_data(nullptr);
        poller->call_soon([=]() {
            logger->debug("request_recv_response: end of closure");
        });
    });
}

void request_sendrecv(Var<Transport> transport, Settings settings,
        Headers headers, std::string body, RequestSendrecvCb callback,
        Poller *poller, Logger *logger) {
    request_send(transport, settings, headers, body, [=](Error error) {
        if (error) {
            callback(error, nullptr);
            return;
        }
        request_recv_response(transport, callback, poller, logger);
    });
}

void request_cycle(Settings settings, Headers headers, std::string body,
        RequestSendrecvCb callback, Poller *poller, Logger *logger) {
    request_connect(settings, [=](Error err, Var<Transport> transport) {
        if (err) {
            callback(err, nullptr);
            return;
        }
        request_sendrecv(transport, settings, headers, body, callback,
                poller, logger);
    }, poller, logger);
}

} // namespace http
} // namespace mk
