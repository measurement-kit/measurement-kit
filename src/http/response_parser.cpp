// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/http/response_parser.hpp"
#include "ext/http-parser/http_parser.h"
#include <functional>
#include <map>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/net.hpp>
#include <stddef.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <type_traits>

extern "C" {

using namespace mk::http;

static int cb_message_begin(http_parser *p) {
    return static_cast<ResponseParserNg *>(p->data)->do_message_begin_();
}

static int cb_status(http_parser *p, const char *s, size_t n) {
    return static_cast<ResponseParserNg *>(p->data)->do_status_(s, n);
}

static int cb_header_field(http_parser *p, const char *s, size_t n) {
    return static_cast<ResponseParserNg *>(p->data)->do_header_field_(s, n);
}

static int cb_header_value(http_parser *p, const char *s, size_t n) {
    return static_cast<ResponseParserNg *>(p->data)->do_header_value_(s, n);
}

static int cb_headers_complete(http_parser *p) {
    return static_cast<ResponseParserNg *>(p->data)->do_headers_complete_();
}

static int cb_body(http_parser *p, const char *s, size_t n) {
    return static_cast<ResponseParserNg *>(p->data)->do_body_(s, n);
}

static int cb_message_complete(http_parser *p) {
    return static_cast<ResponseParserNg *>(p->data)->do_message_complete_();
}

} // extern "C"
namespace mk {
namespace http {

ResponseParserNg::ResponseParserNg(Var<Logger> logger) {
    logger_ = logger;
    http_parser_settings_init(&settings_);
    settings_.on_message_begin = cb_message_begin;
    settings_.on_status = cb_status;
    settings_.on_header_field = cb_header_field;
    settings_.on_header_value = cb_header_value;
    settings_.on_headers_complete = cb_headers_complete;
    settings_.on_body = cb_body;
    settings_.on_message_complete = cb_message_complete;
    http_parser_init(&parser_, HTTP_RESPONSE);
    parser_.data = this; /* Which makes this object non-movable */
}

} // namespace http
} // namespace mk
