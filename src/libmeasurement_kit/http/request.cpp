// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/http/request_impl.hpp"
#include "private/common/utils.hpp"

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

/*static*/ ErrorOr<SharedPtr<Request>> Request::make(Settings settings, Headers headers,
                                               std::string body) {
    SharedPtr<Request> request{std::make_shared<Request>()};
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

void Request::serialize(net::Buffer &buff, SharedPtr<Logger> logger) {
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
        logger->debug2("%s", body.c_str());
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

void request_connect(Settings settings, Callback<Error, SharedPtr<Transport>> txp,
                     SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_connect_impl(settings, txp, reactor, logger);
}

void request_send(SharedPtr<Transport> txp, Settings settings, Headers headers,
                  std::string body, SharedPtr<Logger> logger,
                  Callback<Error, SharedPtr<Request>> callback) {
    request_maybe_send(Request::make(settings, headers, body), txp, logger,
                       callback);
}

void request_maybe_send(ErrorOr<SharedPtr<Request>> request, SharedPtr<Transport> txp,
                        SharedPtr<Logger> logger,
                        Callback<Error, SharedPtr<Request>> callback) {
    if (!request) {
        callback(request.as_error(), {});
        return;
    }
    Buffer buff;
    (*request)->serialize(buff, logger);
    net::write(txp, buff, [=](Error err) {
        callback(err, *request);
    });
}

void request_recv_response(SharedPtr<Transport> txp,
                           Callback<Error, SharedPtr<Response>> cb, Settings settings,
                           SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<ResponseParserNg> parser{std::make_shared<ResponseParserNg>(logger)};
    SharedPtr<Response> response{std::make_shared<Response>()};
    SharedPtr<bool> prevent_emit{std::make_shared<bool>(false)};
    SharedPtr<bool> valid_response{std::make_shared<bool>(false)};

    // Note: any parser error at this point is an exception catched by the
    // connection code and routed to the error handler function below
    txp->on_data([=](Buffer data) { parser->feed(data); });

    parser->on_response([=](Response r) {
        *response = r;
        *valid_response = true;
    });

    ErrorOr<bool> ignore_body = settings.get("http/ignore_body", false);
    if (!ignore_body) {
        cb(ValueError(), response);
        return;
    }
    if (*ignore_body == false) {
        parser->on_body([=](std::string s) {
            response->body += s;
        });
    }

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
        if (response->body.size() > 0) {
            logger->debug2("%s", response->body.c_str());
        }
        txp->emit_error(NoError());
    });
    txp->on_error([=](Error err) {
        logger->debug("http: received error %d on connection", err.code);
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
        txp->on_error(nullptr);
        txp->on_data(nullptr);
        reactor->call_soon([=]() {
            logger->debug2("http: end of closure");
            cb(err, response);
        });
    });
}

void request_sendrecv(SharedPtr<Transport> txp, Settings settings, Headers headers,
                      std::string body, Callback<Error, SharedPtr<Response>> callback,
                      SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_maybe_sendrecv(Request::make(settings, headers, body), txp,
                           callback, settings, reactor, logger);
}

void request_maybe_sendrecv(ErrorOr<SharedPtr<Request>> request, SharedPtr<Transport> txp,
                            Callback<Error, SharedPtr<Response>> callback,
                            Settings settings,
                            SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_maybe_send(request, txp, logger,
                       [=](Error error, SharedPtr<Request> request) {
        if (error) {
            SharedPtr<Response> response{std::make_shared<Response>()};
            response->request = request;
            callback(error, response);
            return;
        }
        request_recv_response(txp, [=](Error error, SharedPtr<Response> response) {
            if (error) {
                callback(error, response);
                return;
            }
            response->request = request;
            callback(error, response);
        }, settings, reactor, logger);
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
             Callback<Error, SharedPtr<Response>> callback, SharedPtr<Reactor> reactor,
             SharedPtr<Logger> logger, SharedPtr<Response> previous, int num_redirs) {
    dump_settings(settings, "request", logger);
    ErrorOr<int> max_redirects = settings.get_noexcept(
        "http/max_redirects", 0
    );
    if (!max_redirects) {
        callback(InvalidMaxRedirectsError(max_redirects.as_error()), {});
        return;
    }
    request_connect(
        settings,
        [=](Error err, SharedPtr<Transport> txp) {
            if (err) {
                callback(err, {});
                return;
            }
            request_sendrecv(
                txp, settings, headers, body,
                [=](Error error, SharedPtr<Response> response) {
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

void request_json_string(
      std::string method, std::string url, std::string data,
      http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_json_string_impl(method, url, data, headers, cb, settings, reactor,
                             logger);
}

void request_json_no_body(
      std::string method, std::string url, http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_json_string(method, url, "", headers, cb, settings, reactor,
                        logger);
}

void request_json_object(
      std::string method, std::string url, Json jdata,
      http::Headers headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb,
      Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    request_json_string(method, url, jdata.dump(), headers, cb, settings,
                        reactor, logger);
}

} // namespace http
} // namespace mk
