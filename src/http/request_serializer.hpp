// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_REQUEST_SERIALIZER_HPP
#define MEASUREMENT_KIT_HTTP_REQUEST_SERIALIZER_HPP

#include <measurement_kit/common/settings.hpp>

#include <measurement_kit/net/buffer.hpp>

#include <measurement_kit/http.hpp>

#include <iosfwd>
#include <string>
#include <utility>

namespace mk {
namespace http {

/*!
 * \brief HTTP request serializer.
 */
struct RequestSerializer {

    std::string method;    /*!< Request method */
    std::string schema;    /*!< URL schema */
    std::string address;   /*!< URL address */
    std::string port;      /*!< URL port */
    std::string pathquery; /*!< URL path followed by optional query */
    std::string protocol;  /*!< Request protocol */
    Headers headers;       /*!< Request headers */
    std::string body;      /*!< Request body */

    /*!
     * \brief Constructor.
     * \param s A std::map with key values of the options supported:
     *
     *             {
     *                 "follow_redirects": "yes|no",
     *                 "url": std::string,
     *                 "ignore_body": "yes|no",
     *                 "method": "GET|DELETE|PUT|POST|HEAD|...",
     *                 "http_version": "HTTP/1.1",
     *                 "path": by default is taken from the url
     *             }
     * \param headers HTTP headers (moved for efficiency).
     * \param body Request body (moved for efficiency).
     */
    RequestSerializer(Settings s, Headers headers, std::string body);

    RequestSerializer() {
        // nothing
    }

    /*!
     * \brief Serialize request.
     * \param buff Buffer where to serialize request.
     */
    void serialize(net::Buffer &buff) {
        buff << method << " " << pathquery << " " << protocol << "\r\n";
        for (auto &kv : headers) {
            buff << kv.first << ": " << kv.second << "\r\n";
        }

        buff << "Host: " << address;
        if (port != "80") {
            buff << ":";
            buff << port;
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
