// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP
#define SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP

#include "../ext/http_parser.h"

#include "src/libmeasurement_kit/common/delegate.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

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
    ResponseParserNg(SharedPtr<Logger> = Logger::global());

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
        logger_->debug2("http: BEGIN");
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
        logger_->debug2("http: STATUS");
        response_.reason.append(s, n);
        return 0;
    }

    int do_header_field_(const char *s, size_t n) {
        logger_->debug2("http: FIELD");
        do_header_internal(HeaderParserState::FIELD, s, n);
        return 0;
    }

    int do_header_value_(const char *s, size_t n) {
        logger_->debug2("http: VALUE");
        do_header_internal(HeaderParserState::VALUE, s, n);
        return 0;
    }

    int do_headers_complete_() {
        logger_->debug2("http: HEADERS_COMPLETE");
        if (field_ != "") { // Also copy last header
            response_.headers[field_] = value_;
        }
        response_.http_major = parser_.http_major;
        response_.status_code = parser_.status_code;
        response_.http_minor = parser_.http_minor;
        std::stringstream sst;
        sst << "HTTP/" << response_.http_major << "." << response_.http_minor
            << " " << response_.status_code << " " << response_.reason;
        response_.response_line = sst.str();
        logger_->debug("< %s", response_.response_line.c_str());
        for (auto kv : response_.headers) {
            logger_->debug("< %s: %s", kv.first.c_str(), kv.second.c_str());
        }
        logger_->debug("<");
        if (response_fn_) {
            response_fn_(response_);
        }
        return 0;
    }

    int do_body_(const char *s, size_t n) {
        logger_->debug2("http: BODY");
        if (body_fn_) {
            body_fn_(std::string(s, n));
        }
        return 0;
    }

    int do_message_complete_() {
        logger_->debug2("http: END");
        if (end_fn_) {
            end_fn_();
        }
        // Rationale: we want to pause the parser after the first message
        // because otherwise, if for whatever reason we receive two messages
        // back to back, only the second will be stored.
        //
        // If this will ever become a limitation because we decide for some
        // reason to reuse the same parser for more than a single response
        // we can simply fix by allowing the caller to unpause us.
        http_parser_pause(&parser_, 1);
        return 0;
    }

  private:
    Delegate<> begin_fn_;
    Delegate<Response> response_fn_;
    Delegate<std::string> body_fn_;
    Delegate<> end_fn_;

    SharedPtr<Logger> logger_ = Logger::global();
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
        // FIX: I initially coded `upgrade` as the following commented out code
        // does, because I did read [the documentation of http-parser](
        // https://github.com/nodejs/http-parser#the-special-problem-of-upgrade)
        // but it seems that is written more with servers in mind.
        //
        // For a client, unless `upgrade` is requested by us, the resulting
        // headers are just informational and, as such, we can continue.
        //
        // For reference see: <https://tools.ietf.org/html/rfc7230#section-6.7>.
        //
        // This fixes for example the `http://aseansec.org/` URL.
        //
        // I am going to keep the code below commented out and this comment
        // so the choices I made are clear and documented.
        //
        // if (parser_.upgrade) {
        //    throw UpgradeError();
        // }
        //
        if (x != n) {
            throw ParserError(map_parser_error_());
        }
        return n;
    }

    Error map_parser_error_() {
        switch (HTTP_PARSER_ERRNO(&parser_)) {
        case HPE_OK:
            return NoError();
        case HPE_INVALID_EOF_STATE:
            return ParserInvalidEofStateError();
        case HPE_HEADER_OVERFLOW:
            return ParserHeaderOverflowError();
        case HPE_CLOSED_CONNECTION:
            return ParserClosedConnectionError();
        case HPE_INVALID_VERSION:
            return ParserInvalidVersionError();
        case HPE_INVALID_STATUS:
            return ParserInvalidStatusError();
        case HPE_INVALID_METHOD:
            return ParserInvalidMethodError();
        case HPE_INVALID_URL:
            return ParserInvalidUrlError();
        case HPE_INVALID_HOST:
            return ParserInvalidHostError();
        case HPE_INVALID_PORT:
            return ParserInvalidPortError();
        case HPE_INVALID_PATH:
            return ParserInvalidPathError();
        case HPE_INVALID_QUERY_STRING:
            return ParserInvalidQueryStringError();
        case HPE_INVALID_FRAGMENT:
            return ParserInvalidFragmentError();
        case HPE_LF_EXPECTED:
            return ParserLfExpectedError();
        case HPE_INVALID_HEADER_TOKEN:
            return ParserInvalidHeaderTokenError();
        case HPE_INVALID_CONTENT_LENGTH:
            return ParserInvalidContentLengthError();
        case HPE_UNEXPECTED_CONTENT_LENGTH:
            return ParserUnexpectedContentLengthError();
        case HPE_INVALID_CHUNK_SIZE:
            return ParserInvalidChunkSizeError();
        case HPE_INVALID_CONSTANT:
            return ParserInvalidConstantError();
        case HPE_INVALID_INTERNAL_STATE:
            return ParserInvalidInternalStateError();
        case HPE_STRICT:
            return ParserStrictModeAssertionError();
        case HPE_PAUSED:
            return ParserPausedError();
        default:
            // FALLTHROUGH
            break;
        };
        return GenericParserError();  /* Should not happen */
    }
};

} // namespace http
} // namespace mk
#endif
