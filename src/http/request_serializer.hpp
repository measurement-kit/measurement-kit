// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_HTTP_REQUEST_SERIALIZER_HPP
#define SRC_HTTP_REQUEST_SERIALIZER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <measurement_kit/http.hpp>
#include <string>
#include <utility>

namespace mk {
namespace http {

class RequestSerializer {
  public:
    std::string method;
    Url url;
    std::string protocol;
    Headers headers;
    std::string path;
    std::string body;

    /*!
     * For settings the following options are supported:
     *
     *     {
     *       {"follow_redirects", "yes|no"},
     *       {"url", std::string},
     *       {"ignore_body", "yes|no"},
     *       {"method", "GET|DELETE|PUT|POST|HEAD|..."},
     *       {"http_version", "HTTP/1.1"},
     *       {"path", by default is taken from the url}
     *     }
     */
    RequestSerializer(Settings settings, Headers hdrs, std::string bd) {
        headers = hdrs;
        body = bd;
        if (settings.find("url") == settings.end()) {
            throw MissingUrlError();
        }
        url = parse_url(settings.at("url"));
        protocol = settings.get("http_version", std::string("HTTP/1.1"));
        method = settings.get("method", std::string("GET"));
        path = settings.get("path", std::string(""));
        if (path != "" && path[0] != '/') {
            path = "/" + path;
        }

    }

    RequestSerializer() {}

    void serialize(net::Buffer &buff) {
        buff << method << " ";
        if (path != "") {
            buff << path;
        } else {
            buff << url.pathquery;
        }
        buff << " " << protocol << "\r\n";
        for (auto &kv : headers) {
            buff << kv.first << ": " << kv.second << "\r\n";
        }

        buff << "Host: " << url.address;
        if (url.port != 80) {
            buff << ":";
            buff << std::to_string(url.port);
        }
        buff << "\r\n";

        if (body != "") {
            buff << "Content-Length: " << std::to_string(body.length())
                 << "\r\n";
        }

        buff << "\r\n";

        if (body != "") {
            buff << body;
        }
    }
};

} // namespace http
} // namespace mk
#endif
