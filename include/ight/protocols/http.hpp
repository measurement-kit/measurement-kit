/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_PROTOCOLS_HTTP_HPP
# define IGHT_PROTOCOLS_HTTP_HPP

#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include <ight/common/constraints.hpp>
#include <ight/common/settings.hpp>
#include <ight/common/log.hpp>
#include <ight/common/error.hpp>
#include <ight/common/pointer.hpp>

#include <ight/net/buffer.hpp>
#include <ight/net/transport.hpp>

// Internally we use joyent/http-parser
struct http_parser;
struct http_parser_settings;

// Internally we use libevent
struct evbuffer;

namespace ight {
namespace protocols {
namespace http {

using namespace ight::common::constraints;
using namespace ight::common::error;
using namespace ight::common::log;
using namespace ight::common::pointer;
using namespace ight::common::settings;

using namespace ight::net;
using namespace ight::net::buffer;
using namespace ight::net::transport;

/*!
 * \brief Raised when the parser receives the UPGRADE method.
 * \remark This should not happen.
 */
struct UpgradeError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/*!
 * \brief Raised when readline() fails because the line is too long.
 */
struct ReadlineError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/*!
 * \brief Raised when a parse error is detected.
 */
struct ParserError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ResponseParserImpl;  // See http.cpp

typedef std::map<std::string, std::string> Headers;

/*!
 * \brief Parse an HTTP response.
 *
 * This class implements an event-style HTTP response parser. The following
 * events are raised: `begin` when the response begins; `headers_complete`
 * when the HTTP headers were received; `body` when a piece of the body was
 * received; `end` when the response body was full received. Multiple
 * responses could be handled by the same parser.
 *
 * Example usage:
 *
 *     auto parser = ight::http::ResponseParser();
 *
 *     parser.on_begin([](void) {
 *         std::clog << "Begin of the response" << std::endl;
 *     });
 *
 *     parser.on_headers_complete([](unsigned short major, unsigned short minor,
 *             unsigned int code, std::string& reason, std::map<std::string,
 *             std::string>&& headers) {
 *         std::clog << "HTTP/" << major << "." << minor
 *                   << code << " " << reason << std::endl;
 *         for (auto pair : headers) {
 *             std::clog << pair.first << ": " << pair.second;
 *         }
 *         std::clog << "=== BEGIN BODY ===" << std::endl;
 *     });
 *
 *     parser.on_body([](std::string&& part) {
 *         std::clog << "body part: " << part << std::endl;
 *     });
 *
 *     parser.on_end([](void) {
 *         std::clog << "=== END BODY ===" << std::endl;
 *     }
 *
 *     ...
 *
 *     auto connection = ight::net::transport::connect(...);
 *
 *     connection.on_data([&](SharedPointer<Buffer> data) {
 *         parser.feed(data);
 *     });
 */
class ResponseParser {

protected:
    ResponseParserImpl *impl = nullptr;

public:
    /*!
     * \brief Default constructor.
     */
    ResponseParser(SharedPointer<Logger> = DefaultLogger::get());

    /*!
     * \brief Deleted copy constructor.
     */
    ResponseParser(ResponseParser& other) = delete;

    /*!
     * \brief Deleted copy assignment.
     */
    ResponseParser& operator=(ResponseParser& other) = delete;

    /*!
     * \brief Move constructor.
     */
    ResponseParser(ResponseParser&& other) {
        std::swap(impl, other.impl);
    }

    /*!
     * \brief Move assignment.
     */
    ResponseParser& operator=(ResponseParser&& other) {
        std::swap(impl, other.impl);
        return *this;
    }

    /*!
     * \brief Destructor.
     */
    ~ResponseParser(void);

    /*!
     * \brief Register `begin` event handler.
     * \param fn The `begin` event handler.
     */
    void on_begin(std::function<void(void)>&& fn);

    /*!
     * \brief Register `headers_complete` event handler.
     * \param fn The `headers_complete` event handler.
     * \remark The parameters received by the event handler are, respectively,
     *         the HTTP major version number, the HTTP minor version number,
     *         the status code, the reason string, and a map containing the
     *         HTTP headers.
     */
    void on_headers_complete(std::function<void(unsigned short http_major,
      unsigned short http_minor, unsigned int status_code, std::string&& reason,
      Headers&& headers)>&& fn);

    /*!
     * \brief Register `body` event handler.
     * \param fn The `body` event handler.
     * \remark The parameter received by the event handler is the received
     *         piece of body.
     * \remark By default this event handler is unset, meaning that the
     *         received body is ignored.
     */
    void on_body(std::function<void(std::string&&)>&& fn);

    /*!
     * \brief Register `end` event handler.
     * \param fn The `end` event handler.
     */
    void on_end(std::function<void(void)>&& fn);

    /*!
     * \brief Feed the parser.
     * \param data Evbuffer containing the received data.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(SharedPointer<Buffer> data);

    /*!
     * \brief Feed the parser.
     * \param data String containing the received data.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(std::string data);

    /*!
     * \brief Feed the parser.
     * \param c Character containing the received data.
     * \remark This function is used for testing.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(char c);

    /*!
     * \brief Tell the parser we hit EOF.
     * \remark This allows us to implement the body-ends-at-EOF semantic.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void eof();
};

/*!
 * \brief HTTP stream. This class implements a HTTP stream, i.e., a
 * connection that filters incoming data using the HTTP protocol.
 *
 * For now this class only filters responses.
 *
 * This is a mid-level HTTP abstraction that can be used when a high
 * degree of control is needed over the HTTP protocol.
 */
class Stream {
    SharedPointer<Transport> connection;
    SharedPointer<ResponseParser> parser;
    std::function<void(Error)> error_handler;
    std::function<void()> connect_handler;

    void connection_ready(void) {
        connection->on_data([&](SharedPointer<Buffer> data) {
            parser->feed(data);
        });
        //
        // Intercept EOF error to implement body-ends-at-EOF semantic.
        // TODO: convert error from integer to exception.
        //
        connection->on_error([&](Error error) {
            auto safe_eh = error_handler;
            if (error.error == 0) {
                parser->eof();
            }
            // parser->eof() may cause this object to go out of
            // the scope, therefore we cannot trust `this` to be
            // valid in the following code.
            if (safe_eh) {
                safe_eh(error);
            }
        });
        connect_handler();
    }

public:

    /*!
     * \brief Get response parser.
     * This is useful for writing tests.
     */
    SharedPointer<ResponseParser> get_parser() {
        return parser;
    }

    SharedPointer<Transport> get_transport() {
        return connection;
    }

    /*!
     * \brief Deleted copy constructor.
     * The `this` of this class is bound to lambdas, so it must
     * not be copied or moved.
     */
    Stream(Stream& /*other*/) = delete;

    /*!
     * \brief Deleted copy assignment.
     * The `this` of this class is bound to lambdas, so it must
     * not be copied or moved.
     */
    Stream& operator=(Stream& /*other*/) = delete;

    /*!
     * \brief Deleted move constructor.
     * The `this` of this class is bound to lambdas, so it must
     * not be copied or moved.
     */
    Stream(Stream&& /*other*/) = delete;

    /*!
     * \brief Deleted move assignment.
     * The `this` of this class is bound to lambdas, so it must
     * not be copied or moved.
     */
    Stream& operator=(Stream&& /*other*/) = delete;

    //
    // How do you construct this object:
    //

    Stream(void) {
        // nothing to do
    }

    /*!
     * \brief Create a stream that connects to a remote host.
     * \param settings Settings passed to the transport to initialize
     *        it (see transport.cpp for more info).
     */
    Stream(Settings settings, SharedPointer<Logger> lp = DefaultLogger::get()) {
        parser = std::make_shared<ResponseParser>(lp);
        connection = transport::connect(settings, lp);
        //
        // While the connection is in progress, just forward the
        // error if needed, we'll deal with body-terminated-by-EOF
        // semantic when we know we are actually connected.
        //
        connection->on_error([&](Error error) {
            if (error_handler) {
                error_handler(error);
            }
        });
    }

    /*!
     * \brief Register callback for "connect" event.
     * \param fn Callback invoked when the connection is established.
     * \remark The callback is not called if you constructed this
     *         object attaching it to an already opened socket.
     */
    void on_connect(std::function<void(void)>&& fn) {
        connect_handler = fn;
        connection->on_connect([this]() {
            connection_ready();
        });
    }

    /*!
     * \brief Close this stream.
     */
    void close() {
        connection->close();
    }

    /*!
     * \brief Destructor.
     */
    ~Stream(void) {
        // Nothing to do here
    }

    //
    // Low-level interface for sending data to the other end:
    //

    /*!
     * \brief Write data on the underlying socket.
     * \param data Data to be sent on the underlying socket.
     * \throws std::runtime_error on error.
     * \returns A reference to this stream for chaining operations.
     */
    Stream& operator<<(std::string data) {
        connection->send(data);
        return *this;
    }

    /*!
     * \brief Register callback for "flush" event.
     * \param fn Callback invoked when the underlying socket is flushed.
     * \remark This callback allows you to control when to send more data
     *         after some data was already passed to the kernel.
     */
    void on_flush(std::function<void(void)>&& fn) {
        connection->on_flush(std::move(fn));
    }

    //
    // Events generated by the HTTP response parser:
    //

    /*!
     * \brief Register `begin` event handler.
     * \param fn The `begin` event handler.
     */
    void on_begin(std::function<void(void)>&& fn) {
        parser->on_begin(std::move(fn));
    }

    /*!
     * \brief Register `headers_complete` event handler.
     * \param fn The `headers_complete` event handler.
     * \remark The parameters received by the event handler are, respectively,
     *         the HTTP major version number, the HTTP minor version number,
     *         the status code, the reason string, and a map containing the
     *         HTTP headers.
     */
    void on_headers_complete(std::function<void(unsigned short,
      unsigned short, unsigned int, std::string&&, Headers&&)>&& fn) {
        parser->on_headers_complete(std::move(fn));
    }

    /*!
     * \brief Register `body` event handler.
     * \param fn The `body` event handler.
     * \remark The parameter received by the event handler is the
     *         piece of body that was just received.
     */
    void on_body(std::function<void(std::string&&)>&& fn) {
        parser->on_body(std::move(fn));
    }

    /*!
     * \brief Register `end` event handler.
     * \param fn The `end` event handler.
     */
    void on_end(std::function<void(void)>&& fn) {
        parser->on_end(std::move(fn));
    }

    /*!
     * \brief Register `error` event handler.
     * \param fn The `error event handler.
     */
    void on_error(std::function<void(Error)>&& fn) {
        error_handler = std::move(fn);
    }

    /*!
     * \brief Set timeout.
     * \param time The timeout in seconds.
     */
    void set_timeout(double timeo) {
        connection->set_timeout(timeo);
    }

    std::string socks5_address() {
        return connection->socks5_address();
    }

    std::string socks5_port() {
        return connection->socks5_port();
    }
};

/*!
 * \brief HTTP request serializer.
 */
struct RequestSerializer {

    std::string method;         /*!< Request method */
    std::string schema;         /*!< URL schema */
    std::string address;        /*!< URL address */
    std::string port;           /*!< URL port */
    std::string pathquery;      /*!< URL path followed by optional query */
    std::string protocol;       /*!< Request protocol */
    Headers headers;            /*!< Request headers */
    std::string body;           /*!< Request body */

    /*!
     * \brief Constructor.
     * \param s A std::map with key values of the options supported:
     *
     *             {
     *                 "follow_redirects": "yes|no",
     *                 "url": std::string,
     *                 "ignore_body": "yes|no",
     *                 "method": "GET|DELETE|PUT|POST|HEAD|...",
     *                 "http_version": "HTTP/1.1",
     *                 "path": by default is taken from the url
     *             }
     * \param headers HTTP headers (moved for efficiency).
     * \param body Request body (moved for efficiency).
     */
    RequestSerializer(Settings s, Headers headers,
                      std::string body);

    RequestSerializer() {
        // nothing
    }

    /*!
     * \brief Serialize request.
     * \param buff Buffer where to serialize request.
     */
    void serialize(Buffer& buff) {
        buff << method << " " << pathquery << " " << protocol << "\r\n";
        for (auto& kv : headers) {
            buff << kv.first << ": " << kv.second << "\r\n";
        }

        buff << "Host: " << address;
        if (port != "80") {
            buff << ":";
            buff << port;
        }
        buff << "\r\n";

        if (body != "") {
            buff << "Content-Length: " << std::to_string(body.length())
                   << "\r\n";
        }

        buff << "\r\n";

        if (body != "") {
            buff << body;
        }
    }
};

/*!
 * \brief HTTP response.
 */
struct Response {
    std::string response_line;      /*!< Original HTTP response line */
    unsigned short http_major;      /*!< HTTP major version number */
    unsigned short http_minor;      /*!< HTTP minor version number */
    unsigned int status_code;       /*!< HTTP status code */
    std::string reason;             /*!< HTTP reason string */
    Headers headers;                /*!< Response headers */
    Buffer body;                    /*!< Response body */
};

class Request;  // Forward declaration

typedef std::function<void(Error, Response&&)> RequestCallback;

/*!
 * \brief HTTP request.
 */
class Request : public NonCopyable, public NonMovable {

    RequestCallback callback;
    RequestSerializer serializer;
    SharedPointer<Stream> stream;
    Response response;
    std::set<Request *> *parent = nullptr;
    SharedPointer<Logger> logger = DefaultLogger::get();

    void emit_end(Error error, Response&& response) {
        close();
        callback(error, std::move(response));
        //
        // Self cleanup when we're owned by a Client.
        //
        // Note: we only detach ourself if we reach the final state, otherwise
        // the Client will manage to detach this object.
        //
        if (parent != nullptr) {
            parent->erase(this);
            delete this;
            return;
        }
    }

public:
    /*!
     * \brief Constructor.
     * \param settings_ A std::map with key values of the options supported:
     *                     {
     *                         "follow_redirects": "yes|no",
     *                         "url": std::string,
     *                         "ignore_body": "yes|no",
     *                         "method": "GET|DELETE|PUT|POST|HEAD|...",
     *                         "http_version": "HTTP/1.1",
     *                         "path": by default is taken from the url
     *                     }
     * \param headers Request headers.
     * \param callback Function invoked when request is complete.
     * \param logger Logger to be used.
     * \param parent Pointer to parent to implement self clean up.
     */
    Request(const Settings settings_, Headers headers,
            std::string body, RequestCallback&& callback_,
            SharedPointer<Logger> lp = DefaultLogger::get(),
            std::set<Request *> *parent_ = nullptr)
                : callback(callback_), parent(parent_), logger(lp) {
        auto settings = settings_;  // Make a copy and work on that
        serializer = RequestSerializer(settings, headers, body);
        // Extend settings with address and port to connect to
        settings["port"] = serializer.port;
        settings["address"] = serializer.address;
        // If needed, extend settings with socks5 proxy info
        if (serializer.schema == "httpo") {
            // tor_socks_port takes precedence because it's more specific
            if (settings.find("tor_socks_port") != settings.end()) {
                std::string proxy = "127.0.0.1:";
                proxy += settings["tor_socks_port"];
                settings["socks5_proxy"] = proxy;
            } else if (settings.find("socks5_proxy") == settings.end()) {
                settings["socks5_proxy"] = "127.0.0.1:9050";
            }
        }
        stream = std::make_shared<Stream>(settings, logger);
        stream->on_error([this](Error err) {
            if (err.error != 0) {
                emit_end(err, std::move(response));
            } else {
                // When EOF is received, on_end() is called, therefore we
                // don't need to call emit_end() again here.
            }
        });
        stream->on_connect([this](void) {
            // TODO: improve the way in which we serialize the request
            //       to reduce unnecessary copies
            Buffer buf;
            serializer.serialize(buf);
            *stream << buf.read<char>();

            stream->on_flush([this]() {
                logger->debug("http: request sent... waiting for response");
            });

            stream->on_headers_complete([&](unsigned short major,
                    unsigned short minor, unsigned int status,
                    std::string&& reason, Headers&& headers) {
                logger->debug("http: headers received...");
                response.http_major = major;
                response.http_minor = minor;
                response.status_code = status;
                response.reason = std::move(reason);
                response.headers = std::move(headers);
            });

            stream->on_body([&](std::string&& chunk) {
                logger->debug("http: received body chunk...");
                // FIXME: I am not sure whether the body callback
                //        is still needed or not...
                response.body << chunk;
            });

            stream->on_end([&]() {
                logger->debug("http: we have reached end of response");
                emit_end(Error(0), std::move(response));
            });

        });
    }

    SharedPointer<Stream> get_stream() {
        return stream;
    }

    void close() {
        stream->close();
    }

    ~Request() {
        close();
    }

    std::string socks5_address() {
        return stream->socks5_address();
    }

    std::string socks5_port() {
        return stream->socks5_port();
    }
};

/*!
 * \brief HTTP client.
 */
class Client {

protected:
    std::set<Request *> pending;

public:
    /*!
     * \brief Issue HTTP request.
     * \see Request::Request.
     */
    void request(Settings settings, Headers headers,
            std::string body, RequestCallback&& callback,
            SharedPointer<Logger> lp = DefaultLogger::get()) {
        auto r = new Request(settings, headers, body,
                std::move(callback), lp, &pending);
        pending.insert(r);
    }

    Client() {
        // nothing to do
    }

    ~Client() {
        for (auto& r: pending) {
            //
            // This calls the stream destructor which deletes every
            // proxy object and possibly delayes the deletion of the
            // real parser and connection, just to avoid running
            // code over already deleted structures.
            //
            delete r;
        }
        pending.clear();
    }

    //
    // TODO: implement all the fancy methods
    //
};

}}}
#endif
