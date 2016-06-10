// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ext/http-parser/http_parser.h"
#include <measurement_kit/http.hpp>

namespace mk {
namespace http {

Url parse_url(std::string url) {
    Url retval;
    http_parser_url url_parser;
    http_parser_url_init(&url_parser);
    if (http_parser_parse_url(url.data(), url.size(), 0, &url_parser) != 0) {
        throw UrlParserError();
    }
    if ((url_parser.field_set & (1 << UF_SCHEMA)) == 0) {
        throw MissingUrlSchemaError();
    }
    if ((url_parser.field_set & (1 << UF_HOST)) == 0) {
        throw MissingUrlHostError();
    }
    retval.schema = url.substr(url_parser.field_data[UF_SCHEMA].off,
                               url_parser.field_data[UF_SCHEMA].len);
    retval.address = url.substr(url_parser.field_data[UF_HOST].off,
                                url_parser.field_data[UF_HOST].len);
    if ((url_parser.field_set & (1 << UF_PORT)) != 0) {
        retval.port = url_parser.port;
    } else if (retval.schema == "https") {
        retval.port = 443;
    } else {
        retval.port = 80; /* redundant; but I want to cover all cases */
    }
    if ((url_parser.field_set & (1 << UF_PATH)) != 0) {
        retval.path = url.substr(url_parser.field_data[UF_PATH].off,
                                 url_parser.field_data[UF_PATH].len);
    } else {
        retval.path = "/";
    }
    retval.pathquery = retval.path;
    if ((url_parser.field_set & (1 << UF_QUERY)) != 0) {
        retval.query = url.substr(url_parser.field_data[UF_QUERY].off,
                                  url_parser.field_data[UF_QUERY].len);
        retval.pathquery += "?" + retval.query;
    }
    return retval;
}

ErrorOr<Url> parse_url_noexcept(std::string url) {
    try {
        return parse_url(url);
    } catch (Error &error) {
        return error;
    }
}

} // namespace http
} // namespace mk
