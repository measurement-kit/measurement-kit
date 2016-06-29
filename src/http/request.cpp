// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/http/request_impl.hpp"

namespace mk {
namespace http {

using namespace mk::net;

/*
      _
  ___| | __ _ ___ ___
 / __| |/ _` / __/ __|
| (__| | (_| \__ \__ \
 \___|_|\__,_|___/___/

*/

Error Request::init(Settings settings, Headers hdrs, std::string bd) {
    headers = hdrs;
    body = bd;
    // TODO: the following check and the URL parsing are duplicated also
    // in the code that connect()s; we should not duplicate code
    if (settings.find("http/url") == settings.end()) {
        return MissingUrlError();
    }
    ErrorOr<Url> maybe_url = parse_url_noexcept(settings.at("http/url"));
    if (!maybe_url) {
        return maybe_url.as_error();
    }
    url = *maybe_url;
    protocol = settings.get("http/http_version", std::string("HTTP/1.1"));
    method = settings.get("http/method", std::string("GET"));
    // XXX should we really distinguish between path and query here?
    path = settings.get("http/path", std::string(""));
    if (path != "" && path[0] != '/') {
        path = "/" + path;
    }
    return NoError();
}

void Request::serialize(net::Buffer &buff) {
    buff << method << " ";
    if (path != "") {
        buff << path;
    } else {
        buff << url.pathquery;
    }
    buff << " " << protocol << "\r\n";
    for (auto kv : headers) {
        buff << kv.first << ": " << kv.second << "\r\n";
    }
    buff << "Host: " << url.address;
    if (url.port != 80) {
        buff << ":";
        buff << std::to_string(url.port);
    }
    buff << "\r\n";
    if (body != "") {
        buff << "Content-Length: " << std::to_string(body.length()) << "\r\n";
    }
    buff << "\r\n";
    if (body != "") {
        buff << body;
    }
}

/*
 _             _
| | ___   __ _(_) ___
| |/ _ \ / _` | |/ __|
| | (_) | (_| | | (__
|_|\___/ \__, |_|\___|
         |___/
*/

void request_connect(Settings settings, Callback<Error, Var<Transport>> txp,
                     Var<Reactor> reactor, Var<Logger> logger) {
    request_connect_impl(settings, txp, reactor, logger);
}

void request_send(Var<Transport> txp, Settings settings, Headers headers,
                  std::string body, Callback<Error> callback) {
    Request request;
    Error error = request.init(settings, headers, body);
    if (error) {
        callback(error);
        return;
    }
    // TODO: here we can simplify by using net::write()
    txp->on_error([txp, callback](Error error) {
        txp->on_error(nullptr);
        txp->on_flush(nullptr);
        callback(error);
    });
    txp->on_flush([txp, callback]() {
        txp->on_error(nullptr);
        txp->on_flush(nullptr);
        callback(NoError());
    });
    Buffer buff;
    request.serialize(buff);
    txp->write(buff);
}

void request_recv_response(Var<Transport> txp,
                           Callback<Error, Var<Response>> cb,
                           Var<Reactor> reactor, Var<Logger> logger) {
    Var<ResponseParserNg> parser(new ResponseParserNg);
    Var<Response> response(new Response);
    Var<bool> prevent_emit(new bool(false));

    txp->on_data([=](Buffer data) { parser->feed(data); });
    parser->on_response([=](Response r) { *response = r; });

    // TODO: here we should honour the `ignore_body` setting
    parser->on_body([=](std::string s) { response->body += s; });

    // TODO: we should improve the parser such that the transport forwards the
    // "error" event to it and then the parser does the right thing, so that the
    // code becomes less "twisted" here.

    parser->on_end([=]() {
        if (*prevent_emit == true) {
            return;
        }
        txp->emit_error(NoError());
    });
    txp->on_error([=](Error err) {
        if (err == EofError()) {
            // Calling parser->on_eof() could trigger parser->on_end() and
            // we don't want this function to call ->emit_error()
            *prevent_emit = true;
            parser->eof();
        }
        txp->on_error(nullptr);
        txp->on_data(nullptr);
        reactor->call_soon([=]() {
            logger->log(MK_LOG_DEBUG2, "request_recv_response: end of closure");
            cb(err, response);
        });
    });
}

void request_sendrecv(Var<Transport> txp, Settings settings, Headers headers,
                      std::string body, Callback<Error, Var<Response>> callback,
                      Var<Reactor> reactor, Var<Logger> logger) {
    request_send(txp, settings, headers, body, [=](Error error) {
        if (error) {
            callback(error, nullptr);
            return;
        }
        request_recv_response(txp, callback, reactor, logger);
    });
}

void request(Settings settings, Headers headers, std::string body,
             Callback<Error, Var<Response>> callback, Var<Reactor> reactor,
             Var<Logger> logger) {
    request_connect(
        settings,
        [=](Error err, Var<Transport> txp) {
            if (err) {
                callback(err, nullptr);
                return;
            }
            request_sendrecv(
                txp, settings, headers, body,
                [callback, txp](Error error, Var<Response> response) {
                    txp->close([callback, error, response]() {
                        callback(error, response);
                    });
                },
                reactor, logger);
        },
        reactor, logger);
}

} // namespace http
} // namespace mk
