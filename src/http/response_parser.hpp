// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_HTTP_RESPONSE_PARSER_HPP
#define SRC_HTTP_RESPONSE_PARSER_HPP

#include "ext/http-parser/http_parser.h"
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/net.hpp>

#include <functional>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace mk {
namespace http {
using namespace mk::net;

enum class HeaderParserState {
    NOTHING = 0,
    FIELD = 1,
    VALUE = 2,
};

class ResponseParserNg : public NonCopyable, public NonMovable {
  public:
    ResponseParserNg(Var<Logger> = Logger::global());

    void on_begin(std::function<void()> fn) { begin_fn_ = fn; }

    void on_response(std::function<void(Response)> fn) { response_fn_ = fn; }

    void on_body(std::function<void(std::string)> fn) { body_fn_ = fn; }

    void on_end(std::function<void()> fn) { end_fn_ = fn; }

    void feed(Buffer &data) {
        buffer_ << data;
        parse();
    }

    void feed(std::string data) {
        buffer_ << data;
        parse();
    }

    void feed(const char c) {
        buffer_.write((const void *)&c, 1);
        parse();
    }

    void eof() { parser_execute(nullptr, 0); }

    int do_message_begin_() {
        logger_->log(MK_LOG_DEBUG2, "http: BEGIN");
        response_ = Response();
        prev_ = HeaderParserState::NOTHING;
        field_ = "";
        value_ = "";
        if (begin_fn_) {
            begin_fn_();
        }
        return 0;
    }

    int do_status_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: STATUS");
        response_.reason.append(s, n);
        return 0;
    }

    int do_header_field_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: FIELD");
        do_header_internal(HeaderParserState::FIELD, s, n);
        return 0;
    }

    int do_header_value_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: VALUE");
        do_header_internal(HeaderParserState::VALUE, s, n);
        return 0;
    }

    int do_headers_complete_() {
        logger_->log(MK_LOG_DEBUG2, "http: HEADERS_COMPLETE");
        if (field_ != "") { // Also copy last header
            response_.headers[field_] = value_;
        }
        response_.http_major = parser_.http_major;
        response_.status_code = parser_.status_code;
        response_.http_minor = parser_.http_minor;
        if (response_fn_) {
            response_fn_(response_);
        }
        return 0;
    }

    int do_body_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: BODY");
        if (body_fn_) {
            body_fn_(std::string(s, n));
        }
        return 0;
    }

    int do_message_complete_() {
        logger_->log(MK_LOG_DEBUG2, "http: END");
        if (end_fn_) {
            end_fn_();
        }
        return 0;
    }

  private:
    Delegate<> begin_fn_;
    Delegate<Response> response_fn_;
    Delegate<std::string> body_fn_;
    Delegate<> end_fn_;

    Var<Logger> logger_ = Logger::global();
    http_parser parser_;
    http_parser_settings settings_;
    Buffer buffer_;

    // Variables used during parsing
    Response response_;
    HeaderParserState prev_ = HeaderParserState::NOTHING;
    std::string field_;
    std::string value_;

    void do_header_internal(HeaderParserState cur, const char *s, size_t n) {
        using HPS = HeaderParserState;
        //
        // This implements the finite state machine described by the
        // documentation of joyent/http-parser.
        //
        // See github.com/joyent/http-parser/blob/master/README.md#callbacks
        //
        if (prev_ == HPS::NOTHING && cur == HPS::FIELD) {
            field_ = std::string(s, n);
        } else if (prev_ == HPS::VALUE && cur == HPS::FIELD) {
            response_.headers[field_] = value_;
            field_ = std::string(s, n);
        } else if (prev_ == HPS::FIELD && cur == HPS::FIELD) {
            field_.append(s, n);
        } else if (prev_ == HPS::FIELD && cur == HPS::VALUE) {
            value_ = std::string(s, n);
        } else if (prev_ == HPS::VALUE && cur == HPS::VALUE) {
            value_.append(s, n);
        } else {
            throw HeaderParserInternalError();
        }
        prev_ = cur;
    }

    // XXX parse() and parser_execute() should return Error
    // and should be tagged with warn_unused_result

    void parse() {
        size_t total = 0;
        buffer_.for_each([&](const void *p, size_t n) {
            total += parser_execute(p, n);
            return true;
        });
        buffer_.discard(total);
    }

    size_t parser_execute(const void *p, size_t n) {
        size_t x =
            http_parser_execute(&parser_, &settings_, (const char *)p, n);
        if (parser_.upgrade) {
            throw UpgradeError();
        }
        if (x != n) {
            throw ParserError();
        }
        return n;
    }
};

} // namespace http
} // namespace mk
#endif
