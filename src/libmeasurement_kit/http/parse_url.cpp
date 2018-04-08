// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ext/http_parser.h"
#include "src/libmeasurement_kit/http/http.hpp"

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
        return {NoError(), parse_url(url)};
    } catch (Error &error) {
        return {error, {}};
    }
}

std::string Url::str() {
    std::stringstream sst;
    sst << schema;
    sst << "://";
    if (net::is_ipv6_addr(address)) {
        sst << "[";
    }
    sst << address;
    if (net::is_ipv6_addr(address)) {
        sst << "]";
    }
    if ((schema == "http" and port != 80) or
        (schema == "https" and port != 443)) {
        sst << ":";
        sst << port;
    }
    if (pathquery != "") {
        sst << pathquery;
    } else {
        sst << "/";
    }
    return sst.str();
}

} // namespace http
} // namespace mk
