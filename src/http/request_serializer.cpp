// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/settings.hpp>

#include <measurement_kit/http/headers.hpp>
#include "src/http/request_serializer.hpp"

#include "ext/http-parser/http_parser.h"

#include <string.h>

#include <iosfwd>
#include <stdexcept>
#include <string>

using namespace measurement_kit::common;
using namespace measurement_kit::net;

namespace measurement_kit {
namespace http {

RequestSerializer::RequestSerializer(Settings settings,
        Headers headers_, std::string body_)
                : headers(headers_), body(body_)
{
    auto url = settings["url"];
    http_parser_url url_parser;
    memset(&url_parser, 0, sizeof (url_parser));
    if (http_parser_parse_url(url.c_str(), url.length(), 0,
                &url_parser) != 0) {
        throw std::runtime_error("Cannot parse input URL");
    }
    if ((url_parser.field_set & (1 << UF_SCHEMA)) == 0) {
        throw std::runtime_error("Missing URL schema");
    }
    if ((url_parser.field_set & (1 << UF_HOST)) == 0) {
        throw std::runtime_error("Missing URL host");
    }
    schema = url.substr(url_parser.field_data[UF_SCHEMA].off,
            url_parser.field_data[UF_SCHEMA].len);
    address = url.substr(url_parser.field_data[UF_HOST].off,
            url_parser.field_data[UF_HOST].len);
    if ((url_parser.field_set & (1 << UF_PATH)) != 0) {
        pathquery += url.substr(url_parser.field_data[UF_PATH].off,
                url_parser.field_data[UF_PATH].len);
    } else {
        pathquery += "/";
    }
    if ((url_parser.field_set & (1 << UF_QUERY)) != 0) {
        pathquery += "?";
        pathquery += url.substr(url_parser.field_data[UF_QUERY].off,
                url_parser.field_data[UF_QUERY].len);
    }
    if ((url_parser.field_set & (1 << UF_PORT)) != 0) {
        port += url.substr(url_parser.field_data[UF_PORT].off,
                url_parser.field_data[UF_PORT].len);
    } else {
        port += "80";
    }
    protocol = settings["http_version"];
    if (protocol == "") {
        protocol = "HTTP/1.1";
    }
    method = settings["method"];
    if (method == "") {
        method = "GET";
    }
}

} // namespace http
} // namespace measurement_kit
