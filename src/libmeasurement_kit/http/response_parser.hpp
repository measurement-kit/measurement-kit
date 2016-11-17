// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP
#define SRC_LIBMEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP

#include "../ext/http_parser.h"

#include <measurement_kit/http.hpp>

#include <cassert>
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

    // TODO: Now that `parse()` is public, it would probably be more clean
    // to rename `feed()` to `feed_and_parse()`.

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
        size_t count = 0;
        Error err = parser_execute(nullptr, 0, count);
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
        logger_->log(MK_LOG_DEBUG2, "http: paused after parsing headers");
        http_parser_pause(&parser_, 1);
        parser_state_error_ = PausedAfterParsingHeadersError();
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

    // TODO: move this function along with other public functions
  public:
    Error parse() {
        size_t total = 0;
        Error err = NoError();
        buffer_.for_each([&](const void *p, size_t n) {
            size_t count = 0;
            err = parser_execute(p, n, count);
            logger_->log(MK_LOG_DEBUG2, "http: parsed = %ld, err = %s",
                         count, err.explain().c_str());
            total += count;
            return !err;
        });
        buffer_.discard(total);
        if (err) {
            logger_->debug("http: parser failed: %s", err.explain().c_str());
            return err;
        }
        return parser_state_error_;
    }

  private:
    Error parser_execute(const void *p, size_t n, size_t &rv) {
        if (parser_state_error_ == PausedAfterParsingHeadersError()) {
            logger_->log(MK_LOG_DEBUG2, "http: resuming parsing after headers");
            parser_state_error_ = ParsingBodyInProgressError();
            http_parser_pause(&parser_, 0);
            // FALLTHROUGH
        }
        rv = http_parser_execute(&parser_, &settings_, (const char *)p, n);
        if (parser_.upgrade) {
            return UpgradeError();
        }
        assert(rv <= n);
        if (rv < n) {
            return map_parser_error_();
        }
        return NoError();
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
            /*
             * If the parser is paused for a specific reason, use the more
             * specific reason instead of using the generic reason.
             */
            if (parser_state_error_ == PausedAfterParsingHeadersError()) {
                return parser_state_error_;
            }
            return ParserPausedError();
        default:
            // FALLTHROUGH
            break;
        }
        return GenericParserError();
    }
};

} // namespace http
} // namespace mk
#endif
