/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "common/log.h"

#include "net/connection.h"
#include "net/http.hpp"
#include "net/buffer.hpp"

#include "ext/http-parser/http_parser.h"

//
// ResponseParserImpl
//

#define MAXLINE 4096

namespace ight {
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
        // See <http://goo.gl/yA5K8B>  (github.com/joyent/http-parser/...)
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
        buffer.foreach([&](evbuffer_iovec *iov) {
            size_t n = http_parser_execute(&parser, &settings,
                (const char *) iov->iov_base, iov->iov_len);
            if (parser.upgrade) {
                throw ight::http::UpgradeError("Unexpected UPGRADE");
            }
            if (n != iov->iov_len) {
                throw ight::http::ParserError("Parser error");
            }
            total += iov->iov_len;
            return true;
        });
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
};

}}  // namespaces

//
// ResponseParser
//

ight::http::ResponseParser::ResponseParser(void)
{
    impl = new ight::http::ResponseParserImpl();
}

ight::http::ResponseParser::~ResponseParser(void)
{
    if (impl == nullptr) {
        return;
    }
    delete impl;
    impl = nullptr;  // Idempotent
}

void
ight::http::ResponseParser::on_begin(std::function<void(void)>&& fn)
{
    impl->begin_fn = std::move(fn);
}

void
ight::http::ResponseParser::on_headers_complete(std::function<void(
    unsigned short, unsigned short, unsigned int, std::string&&,
    std::map<std::string, std::string>&&)>&& fn)
{
    impl->headers_complete_fn = std::move(fn);
}

void
ight::http::ResponseParser::on_body(std::function<void(std::string&&)>&& fn)
{
    impl->body_fn = std::move(fn);
}

void
ight::http::ResponseParser::on_end(std::function<void(void)>&& fn)
{
    impl->end_fn = std::move(fn);
}

void
ight::http::ResponseParser::feed(evbuffer *data)
{
    impl->feed(data);
}

void
ight::http::ResponseParser::feed(std::string data)
{
    impl->feed(data);
}

void
ight::http::ResponseParser::feed(char c)
{
    impl->feed(c);
}

//
// HTTP Stream
//

namespace ight {
namespace http {

/*!
 * \brief Implementation of ight::http::Stream.
 * \remark This object is not movable.
 */
class StreamImpl {
    IghtConnection connection;
    ResponseParser parser;

  public:

    /*!
     * \brief Deleted copy constructor.
     */
    StreamImpl(StreamImpl& /*other*/) = delete;

    /*!
     * \brief Deleted assignment constructor.
     */
    StreamImpl& operator=(StreamImpl& /*other*/) = delete;

    /*!
     * \brief Deleted move constructor.
     */
    StreamImpl(StreamImpl&& /*other*/) = delete;

    /*!
     * \brief Deleted move assignment.
     */
    StreamImpl& operator=(StreamImpl&& /*other*/) = delete;

    /*!
     * \brief Construct a stream that connects to a remote endpoint.
     * \see Stream::Stream().
     */
    StreamImpl(std::string address, std::string port,
            std::string family) {
        // TODO: change the constructor of IghtConnection
        connection = IghtConnection(family.c_str(), address.c_str(),
                                    port.c_str());
    }

    /*!
     * \brief Construct a stream that attaches to an already connected socket.
     * \see Stream::Stream().
     */
    StreamImpl(evutil_socket_t sock) {
        connection = IghtConnection(sock);
    }

    /*!
     * \brief Register callback for "connect" event.
     * \param fn Callback invoked when the connection is established.
     * \remark The callback is not called if you constructed this
     *         object attaching it to an already opened socket.
     */
    void on_connect(std::function<void(void)>&& fn) {
        connection.on_connect(std::move(fn));
    }

    /*!
     * \brief Write data on the underlying socket.
     * \param data Data to be sent on the underlying socket.
     * \throws std::runtime_error on error.
     * \returns A reference to this stream for chaining operations.
     */
    StreamImpl& operator<<(std::string s) {
        // TODO: change connection to accept a string in input
        if (connection.puts(s.c_str()) != 0) {
            throw std::runtime_error("Cannot write into the connection");
        }
        return *this;
    }

    /*!
     * \brief Register callback for "drain" event.
     * \param fn Callback invoked when the underlying socket is drained.
     * \remark This callback allows you to control when to send more data
     *         after some data was already passed to the kernel.
     */
    void on_flush(std::function<void(void)>&& fn) {
        connection.on_flush(std::move(fn));
    }

    /*!
     * \brief Register `begin` event handler.
     * \param fn The `begin` event handler.
     */
    void on_begin(std::function<void(void)>&& fn) {
        parser.on_begin(std::move(fn));
    }

    /*!
     * \brief Register `headers_complete` event handler.
     * \param fn The `headers_complete` event handler.
     * \remark The parameters received by the event handler are, respectively,
     *         the HTTP major version number, the HTTP minor version number,
     *         the status code, the reason string, and a map containing the
     *         HTTP headers.
     */
    void on_headers_complete(std::function<void(unsigned short, unsigned short,
            unsigned int, std::string&&, std::map<std::string,
            std::string>&&)>&& fn) {
        parser.on_headers_complete(std::move(fn));
    }

    /*!
     * \brief Register `body` event handler.
     * \param fn The `body` event handler.
     * \remark The parameter received by the event handler is the received
     *         piece of body.
     * \remark By default this event handler is unset, meaning that the
     *         received body is ignored.
     */
    void on_body(std::function<void(std::string&&)>&& fn) {
        parser.on_body(std::move(fn));
    }

    /*!
     * \brief Register `end` event handler.
     * \param fn The `end` event handler.
     */
    void on_end(std::function<void(void)>&& fn) {
        parser.on_end(std::move(fn));
    }
};

}}  // namespaces

ight::http::Stream::Stream(std::string address, std::string port,
        std::string family)
{
    impl = new ight::http::StreamImpl(address, port, family);
}

ight::http::Stream::Stream(evutil_socket_t sock)
{
    impl = new ight::http::StreamImpl(sock);
}

void
ight::http::Stream::on_connect(std::function<void(void)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_connect(std::move(fn));
}

ight::http::Stream::~Stream(void)
{
    delete impl;        /* delete handles nullptr */
}

ight::http::Stream&
ight::http::Stream::operator<<(std::string s)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    *impl << s;
    return *this;
}

void
ight::http::Stream::on_flush(std::function<void(void)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_flush(std::move(fn));
}

void
ight::http::Stream::on_begin(std::function<void(void)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_begin(std::move(fn));
}

void
ight::http::Stream::on_headers_complete(std::function<void(
        unsigned short http_major, unsigned short http_minor,
        unsigned int status_code, std::string&& reason,
        std::map<std::string, std::string>&& headers)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_headers_complete(std::move(fn));
}

void
ight::http::Stream::on_body(std::function<void(std::string&&)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_body(std::move(fn));
}

void
ight::http::Stream::on_end(std::function<void(void)>&& fn)
{
    if (impl == nullptr) {
        throw std::runtime_error("Internal pointer is NULL");
    }
    impl->on_end(std::move(fn));
}
