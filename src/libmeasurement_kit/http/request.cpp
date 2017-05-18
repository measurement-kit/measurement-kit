// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../http/request_impl.hpp"
#include "../common/utils.hpp"

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

/*static*/ ErrorOr<Var<Request>> Request::make(Settings settings, Headers headers,
                                               std::string body) {
    Var<Request> request(new Request);
    Error error = request->init(settings, headers, body);
    // Note: the following cannot be simplified using short circuit
    // evaluation because two different types are returned
    if (error) {
        return error;
    }
    return request;
}

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
    url_path = settings.get("http/path", std::string(""));
    if (url_path != "" && url_path[0] != '/') {
        url_path = "/" + url_path;
    }
    return NoError();
}

void Request::serialize(net::Buffer &buff, Var<Logger> logger) {
    buff << method << " ";
    if (url_path != "") {
        buff << url_path;
    } else {
        buff << url.pathquery;
    }
    buff << " " << protocol << "\r\n";
    for (auto kv : headers) {
        buff << kv.first << ": " << kv.second << "\r\n";
    }
    // if the host: header is passed explicitly,
    // don't construct it again here.
    if (headers.find("host") == headers.end()) {
        buff << "Host: " << url.address;
        if ((url.schema == "http" and url.port != 80) or
            (url.schema == "https" and url.port != 443)) {
            buff << ":";
            buff << std::to_string(url.port);
        }
        buff << "\r\n";
    }
    if (body != "") {
        buff << "Content-Length: " << std::to_string(body.length()) << "\r\n";
    }
    buff << "\r\n";
    for (auto s: mk::split(buff.peek(), "\r\n")) {
        logger->debug("> %s", s.c_str());
    }
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
                  std::string body, Callback<Error, Var<Request>> callback) {
    request_maybe_send(Request::make(settings, headers, body), txp, callback);
}

void request_maybe_send(ErrorOr<Var<Request>> request, Var<Transport> txp,
                        Callback<Error, Var<Request>> callback) {
    if (!request) {
        callback(request.as_error(), nullptr);
        return;
    }
    Buffer buff;
    (*request)->serialize(buff);
    net::write(txp, buff, [=](Error err) {
        callback(err, *request);
    });
}

void request_recv_response(Var<Transport> txp,
                           Callback<Error, Var<Response>> cb,
                           Var<Reactor> reactor, Var<Logger> logger) {
    Var<ResponseParserNg> parser(new ResponseParserNg);
    Var<Response> response(new Response);
    Var<bool> prevent_emit(new bool(false));
    Var<bool> valid_response(new bool(false));

    // Note: any parser error at this point is an exception catched by the
    // connection code and routed to the error handler function below
    txp->on_data([=](Buffer data) { parser->feed(data); });

    parser->on_response([=](Response r) {
        *response = r;
        *valid_response = true;
    });

    // TODO: here we should honour the `ignore_body` setting
    parser->on_body([=](std::string s) { response->body += s; });

    // TODO: we should improve the parser such that the transport forwards the
    // "error" event to it and then the parser does the right thing, so that the
    // code becomes less "twisted" here.
    //
    // XXX actually trying to do that could make things worse as we have
    // seen in #690; we should refactor this code with care.

    parser->on_end([=]() {
        if (*prevent_emit == true) {
            return;
        }
        txp->emit_error(NoError());
    });
    txp->on_error([=](Error err) {
        logger->debug("Received error %s on connection",
                      err.as_ooni_error().c_str());
        if (err == EofError() && *valid_response == true) {
            // Calling parser->on_eof() could trigger parser->on_end() and
            // we don't want this function to call ->emit_error()
            *prevent_emit = true;
            // Assume there was no error. The parser will tell us if that
            // is true (it was in final state) or false.
            err = NoError();
            try {
                logger->debug("Now passing EOF to parser");
                parser->eof();
            } catch (Error &second_error) {
                logger->warn("Parsing error at EOF: %d", second_error.code);
                err = second_error;
                // FALLTHRU
            }
        }
        logger->debug("Now reacting to delivered error %d", err.code);
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
    request_maybe_sendrecv(Request::make(settings, headers, body), txp,
                           callback, reactor, logger);
}

void request_maybe_sendrecv(ErrorOr<Var<Request>> request, Var<Transport> txp,
                            Callback<Error, Var<Response>> callback,
                            Var<Reactor> reactor, Var<Logger> logger) {
    request_maybe_send(request, txp, [=](Error error, Var<Request> request) {
        if (error) {
            Var<Response> response{new Response};
            response->request = request;
            callback(error, response);
            return;
        }
        request_recv_response(txp, [=](Error error, Var<Response> response) {
            if (error) {
                callback(error, response);
                return;
            }
            response->request = request;
            callback(error, response);
        }, reactor, logger);
    });
}

ErrorOr<Url> redirect(const Url &orig_url, const std::string &location) {
    std::stringstream ss;
    /*
     * Note: RFC 1808 Sect. 2.2 is clear that "//"
     * MUST be treated differently than "/".
     */
    if (mk::startswith(location, "//")) {
        ss << orig_url.schema << ":" << location;
    } else if (mk::startswith(location, "/")) {
        Url new_url = orig_url;
        new_url.pathquery = location;
        ss << new_url.str();
    } else if (mk::startswith(location, "http://") ||
               mk::startswith(location, "https://")) {
        ss << location;
    } else {
        /*
         * Assume it's a relative redirect. I seem to recall this should
         * not happen, but it really looks like it happens.
         */
         Url new_url = orig_url;
         if (!mk::endswith(new_url.path, "/")) {
            new_url.path += "/";
         }
         /*
          * Note: clearing the query because a new query may be included in
          * location and keeping also the old query would break things.
          */
         new_url.path += location;
         new_url.query = "";
         // TODO: can we make `pathquery` a property?
         new_url.pathquery = new_url.path;
         ss << new_url.str();
    }
    return parse_url_noexcept(ss.str());
}

void request(Settings settings, Headers headers, std::string body,
             Callback<Error, Var<Response>> callback, Var<Reactor> reactor,
             Var<Logger> logger, Var<Response> previous, int num_redirs) {
    dump_settings(settings, "http::request", logger);
    ErrorOr<int> max_redirects = settings.get_noexcept(
        "http/max_redirects", 0
    );
    if (!max_redirects) {
        callback(InvalidMaxRedirectsError(max_redirects.as_error()), nullptr);
        return;
    }
    request_connect(
        settings,
        [=](Error err, Var<Transport> txp) {
            if (err) {
                callback(err, nullptr);
                return;
            }
            request_sendrecv(
                txp, settings, headers, body,
                [=](Error error, Var<Response> response) {
                    txp->close([=]() {
                        if (error) {
                            callback(error, response);
                            return;
                        }
                        response->previous = previous;
                        if (response->status_code / 100 == 3 and
                            *max_redirects > 0) {
                            logger->debug("following redirect...");
                            std::string loc = response->headers["Location"];
                            if (loc == "") {
                                callback(EmptyLocationError(), response);
                                return;
                            }
                            ErrorOr<Url> url = redirect(
                                response->request->url,
                                loc
                            );
                            if (!url) {
                                callback(InvalidRedirectUrlError(
                                         url.as_error()), response);
                                return;
                            }
                            Settings new_settings = settings;
                            new_settings["http/url"] = url->str();
                            logger->debug("redir url: %s", url->str().c_str());
                            if (num_redirs >= *max_redirects) {
                                callback(TooManyRedirectsError(), response);
                                return;
                            }
                            reactor->call_soon([=]() {
                                request(new_settings, headers, body, callback,
                                    reactor, logger, response, num_redirs + 1);
                            });
                            return;
                        }
                        callback(NoError(), response);
                    });
                },
                reactor, logger);
        },
        reactor, logger);
}

bool HeadersComparator::operator() (
        const std::string &l, const std::string &r) const {
    return strcasecmp(l.c_str(), r.c_str()) < 0;
}

} // namespace http
} // namespace mk
