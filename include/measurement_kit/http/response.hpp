// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_RESPONSE_HPP
#define MEASUREMENT_KIT_HTTP_RESPONSE_HPP

#include <measurement_kit/http/headers.hpp>

#include <iosfwd>
#include <string>

namespace mk {
namespace http {

/*!
 * \brief HTTP response.
 */
struct Response {
    std::string response_line; /*!< Original HTTP response line */
    unsigned short http_major; /*!< HTTP major version number */
    unsigned short http_minor; /*!< HTTP minor version number */
    unsigned int status_code;  /*!< HTTP status code */
    std::string reason;        /*!< HTTP reason string */
    Headers headers;           /*!< Response headers */
    std::string body;          /*!< Response body */
};

} // namespace http
} // namespace mk
#endif
