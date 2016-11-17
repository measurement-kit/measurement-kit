// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP
#define SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP

#include "../ext/http_parser.h"

#include <measurement_kit/http.hpp>

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

    Error feed(Buffer &data) {
        buffer_ << data;
        return parse();
    }

    Error feed(std::string data) {
        buffer_ << data;
        return parse();
    }

    Error feed(const char c) {
        buffer_.write((const void *)&c, 1);
        return parse();
    }

    Error eof() {
        Error err = parser_execute(nullptr, 0).as_error();
        if (err) {
            logger_->debug("http: parser failed at EOF: %s",
                           err.explain().c_str());
            return err;
        }
        return parser_state_error_;
    }

    int do_message_begin_() {
        logger_->log(MK_LOG_DEBUG2, "http: BEGIN");
        response = Response();
        prev_ = HeaderParserState::NOTHING;
        field_ = "";
        value_ = "";
        parser_state_error_ = ParsingHeadersInProgressError();
        return 0;
    }

    int do_status_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: STATUS");
        response.reason.append(s, n);
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
            response.headers[field_] = value_;
        }
        response.http_major = parser_.http_major;
        response.status_code = parser_.status_code;
        response.http_minor = parser_.http_minor;
        std::stringstream sst;
        sst << "HTTP/" << response.http_major << "." << response.http_minor
            << " " << response.status_code << " " << response.reason;
        response.response_line = sst.str();
        logger_->debug("< %s", response.response_line.c_str());
        for (auto kv : response.headers) {
            logger_->debug("< %s: %s", kv.first.c_str(), kv.second.c_str());
        }
        parser_state_error_ = ParsingBodyInProgressError();
        // TODO: pause right here to allow streaming parser
        return 0;
    }

    int do_body_(const char *s, size_t n) {
        logger_->log(MK_LOG_DEBUG2, "http: BODY");
        // TODO: allow to skip the body
        response.body += std::string(s, n);
        return 0;
    }

    int do_message_complete_() {
        logger_->log(MK_LOG_DEBUG2, "http: END");
        /*
         * Pause the parser such that further data, e.g. another response, is
         * not parsed overwriting what has been parsed so far.
         *
         * XXX explain how this should work.
         */
        http_parser_pause(&parser_, 1);
        parser_state_error_ = NoError();
        return 0;
    }

    // Variables that you can access to fetch parsers' results
    Response response;

  private:
    Var<Logger> logger_ = Logger::global();
    http_parser parser_;
    http_parser_settings settings_;
    Buffer buffer_;
    Error parser_state_error_ = ParsingHeadersInProgressError();

    // Variables used during parsing
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
            response.headers[field_] = value_;
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

    Error parse() {
        size_t total = 0;
        Error err = NoError();
        buffer_.for_each([&](const void *p, size_t n) {
            ErrorOr<size_t> count = parser_execute(p, n);
            if (!count) {
                err = count.as_error();
                return false;
            }
            total += *count;
            return true;
        });
        buffer_.discard(total);
        if (err) {
            logger_->debug("http: parser failed: %s", err.explain().c_str());
            return err;
        }
        return parser_state_error_;
    }

    ErrorOr<size_t> parser_execute(const void *p, size_t n) {
        const char *s = (const char *)p;
        size_t x = http_parser_execute(&parser_, &settings_, s, n);
        if (parser_.upgrade) {
            return UpgradeError();
        }
        if (x != n) {
            return ParserError();
        }
        return n;
    }
};

} // namespace http
} // namespace mk
#endif
