/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "common/log.h"

#include "protocols/http.hpp"
#include "net/buffer.hpp"

#include "ext/http-parser/http_parser.h"

//
// ResponseParserImpl
//

#define MAXLINE 4096

using namespace ight::protocols::http;

namespace ight {
namespace protocols {
namespace http {

/*!
 * \brief Implementation of ResponseParser.
 * \see ResponseParser.
 */
class ResponseParserImpl {

    http_parser parser;
    http_parser_settings settings;
    IghtBuffer buffer;

    // Header parsing states (see do_header_internal)
#define S_NOTHING 0
#define S_FIELD 1
#define S_VALUE 2

    // Variables used during parsing
    std::string reason;
    unsigned int prev = S_NOTHING;
    std::string field;
    std::string value;
    std::map<std::string, std::string> headers;
    bool closing = false;
    bool parsing = false;

    static int do_message_begin(http_parser *p) {
        ight_debug("http: BEGIN");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        impl->reason = "";
        impl->prev = S_NOTHING;
        impl->field = "";
        impl->value = "";
        impl->headers.clear();
        impl->begin_fn();
        return 0;
    }

    static int do_status(http_parser *p, const char *s, size_t n) {
        ight_debug("http: STATUS");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        impl->reason.append(s, n);
        return 0;
    }

    void do_header_internal(unsigned int cur, const char *s, size_t n) {
        //
        // This implements the finite state machine described by the
        // documentation of joyent/http-parser.
        //
        // See github.com/joyent/http-parser/blob/master/README.md#callbacks
        //
        if (prev == S_NOTHING && cur == S_FIELD) {
            field = std::string(s, n);
        } else if (prev == S_VALUE && cur == S_FIELD) {
            headers[std::move(field)] = std::move(value);
            field = std::string(s, n);
        } else if (prev == S_FIELD && cur == S_FIELD) {
            field.append(s, n);
        } else if (prev == S_FIELD && cur == S_VALUE) {
            value = std::string(s, n);
        } else if (prev == S_VALUE && cur == S_VALUE) {
            value.append(s, n);
        } else {
            throw std::runtime_error("Internal error");
        }
        prev = cur;
    }

    static int do_header_field(http_parser *p, const char *s, size_t n) {
        ight_debug("http: FIELD");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        impl->do_header_internal(S_FIELD, s, n);
        return 0;
    }

    static int do_header_value(http_parser *p, const char *s, size_t n) {
        ight_debug("http: VALUE");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        impl->do_header_internal(S_VALUE, s, n);
        return 0;
    }

    static int do_headers_complete(http_parser *p) {
        ight_debug("http: HEADERS_COMPLETE");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        if (impl->field != "") {  // Also copy last header
            impl->headers[std::move(impl->field)] = std::move(impl->value);
        }
        impl->headers_complete_fn(impl->parser.http_major,
            impl->parser.http_minor, impl->parser.status_code,
            std::move(impl->reason), std::move(impl->headers));
        return 0;
    }

    static int do_body(http_parser *p, const char *s, size_t n) {
        ight_debug("http: BODY");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        //
        // By default the body handler is not set. This is to avoid
        // copying (s, n) into a string when you don't want to see
        // the response body, for efficiency.
        //
        if (impl->body_fn) {
            impl->body_fn(std::string(s, n));
        }
        return 0;
    }

    static int do_message_complete(http_parser *p) {
        ight_debug("http: END");
        auto impl = static_cast<ResponseParserImpl *>(p->data);
        impl->end_fn();
        return 0;
    }

    void parse(void) {
        auto total = (size_t) 0;
        parsing = true;
        buffer.foreach([&](evbuffer_iovec *iov) {
            size_t n = http_parser_execute(&parser, &settings,
                (const char *) iov->iov_base, iov->iov_len);
            if (parser.upgrade) {
                throw UpgradeError("Unexpected UPGRADE");
            }
            if (n != iov->iov_len) {
                throw ParserError("Parser error");
            }
            total += iov->iov_len;
            return true;
        });
        parsing = false;
        if (closing) {
            delete this;
            return;
        }
        buffer.discard(total);
    }

public:

    /*!
     * \brief Default constructor.
     */
    ResponseParserImpl(void) {
        memset(&settings, 0, sizeof (settings));
        settings.on_message_begin = do_message_begin;
        settings.on_status = do_status;
        settings.on_header_field = do_header_field;
        settings.on_header_value = do_header_value;
        settings.on_headers_complete = do_headers_complete;
        settings.on_body = do_body;
        settings.on_message_complete = do_message_complete;
        memset(&parser, 0, sizeof (parser));
        http_parser_init(&parser, HTTP_RESPONSE);
        parser.data = this;  /* Which makes this object non-movable */
    }

    /*!
     * \brief Deleted copy constructor.
     */
    ResponseParserImpl(ResponseParserImpl& other) = delete;

    /*!
     * \brief Deleted copy assignment operator.
     */
    ResponseParserImpl& operator=(ResponseParserImpl& other) = delete;

    /*!
     * \brief Deleted move operator.
     */
    ResponseParserImpl(ResponseParserImpl&& other) = delete;

    /*!
     * \brief Deleted move assignment operator.
     */
    ResponseParserImpl& operator=(ResponseParserImpl&& other) = delete;

    /*!
     * \brief Handler for the `begin` event.
     */
    std::function<void(void)> begin_fn = [](void) {
        // nothing
    };

    /*!
     * \brief Handler for the `headers_complete` event.
     * \see RequestParser::on_headers_complete.
     */
    std::function<void(unsigned short, unsigned short, unsigned int,
        std::string&&, std::map<std::string, std::string>&&)>
        headers_complete_fn = [](unsigned short, unsigned short,
        unsigned int, std::string&&, std::map<std::string, std::string>&&) {
        // nothing
    };

    /*!
     * \brief Handler for the `body` event.
     * \see RequestParser::on_body.
     */
    std::function<void(std::string&&)> body_fn;

    /*!
     * \brief Handler for the `end` event.
     */
    std::function<void(void)> end_fn = [](void) {
        // nothing
    };

    /*!
     * \brief Feed the parser.
     * \param data Evbuffer containing the received data.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(evbuffer *data) {
        buffer << data;
        parse();
    }

    /*!
     * \brief Feed the parser.
     * \param data String containing the received data.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(std::string data) {
        buffer << data;
        parse();
    }

    /*!
     * \brief Feed the parser.
     * \param c Character containing the received data.
     * \remark This function is used for testing.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(char c) {
        buffer.write((const void *) &c, 1);
        parse();
    }

    /*!
     * \brief Destroy this parser.
     * \remark Actual destruction may be delayed if parser is currently
     *         parsing any incoming data.
     */
    void destroy() {
        if (closing) {
            return;
        }
        closing = true;
        if (parsing) {
            return;
        }
        delete this;
    }
};

}}}  // namespaces

//
// ResponseParser
//

ResponseParserImpl *
ResponseParser::get_impl()
{
    if (impl == nullptr) {
        impl = new ResponseParserImpl();
    }
    return impl;
}

ResponseParser::~ResponseParser(void)
{
    if (impl == nullptr) {
        return;
    }
    impl->destroy();
    impl = nullptr;  // Idempotent
}

void
ResponseParser::on_begin(std::function<void(void)>&& fn)
{
    get_impl()->begin_fn = std::move(fn);
}

void
ResponseParser::on_headers_complete(std::function<void(
    unsigned short, unsigned short, unsigned int, std::string&&,
    std::map<std::string, std::string>&&)>&& fn)
{
    get_impl()->headers_complete_fn = std::move(fn);
}

void
ResponseParser::on_body(std::function<void(std::string&&)>&& fn)
{
    get_impl()->body_fn = std::move(fn);
}

void
ResponseParser::on_end(std::function<void(void)>&& fn)
{
    get_impl()->end_fn = std::move(fn);
}

void
ResponseParser::feed(evbuffer *data)
{
    get_impl()->feed(data);
}

void
ResponseParser::feed(std::string data)
{
    get_impl()->feed(data);
}

void
ResponseParser::feed(char c)
{
    get_impl()->feed(c);
}
