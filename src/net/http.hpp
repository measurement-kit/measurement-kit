/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_HTTP_HPP
# define LIBIGHT_NET_HTTP_HPP

#include <event2/util.h>  /* XXX For evutil_socket_t */

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

// Internally we use joyent/http-parser
struct http_parser;
struct http_parser_settings;

// Internally we use libevent
struct evbuffer;

namespace ight {
namespace http {

typedef Headers std::map<std::string, std::string>;
typedef Options std::map<std::string, std::string>;

class ResponseParserImpl;  // See http.cpp

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
 *     auto connection = IghtConnection(...);
 *
 *     connection.on_data([&](evbuffer *data) {
 *         parser.feed(data);
 *     });
 */
class ResponseParser {
    ResponseParserImpl *impl = nullptr;

public:

    /*!
     * \brief Default constructor.
     */
    ResponseParser(void);

    /*!
     * \brief Deleted copy constructor.
     */
    ResponseParser(ResponseParser& other) = delete;

    /*!
     * \brief Deleted copy assignment.
     */
    ResponseParser& operator=(ResponseParser& other) = delete;

    /*!
     * \brief Default move constructor.
     */
    ResponseParser(ResponseParser&& other) = default;

    /*!
     * \brief Default move assignment.
     */
    ResponseParser& operator=(ResponseParser&& other) = default;

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
      std::map<std::string, std::string>&& headers)>&& fn);

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
    void feed(evbuffer *data);

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
};

class StreamImpl;  // see http.cpp

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
    StreamImpl *impl = nullptr;

public:

    /*!
     * \brief Deleted copy constructor.
     */
    Stream(Stream& /*other*/) = delete;

    /*!
     * \brief Deleted copy assignment.
     */
    Stream& operator=(Stream& /*other*/) = delete;

    /*!
     * \brief Default move constructor.
     */
    Stream(Stream&& /*other*/) = default;

    /*!
     * \brief Default move assignment.
     */
    Stream& operator=(Stream&& /*other*/) = default;

    //
    // How do you construct this object:
    //

    Stream(void) {
        // nothing to do
    }

    /*!
     * \brief Create a stream that connects to a remote host.
     * \param address Address to connect to (this can either be an
     *        IPv4 address, and IPv6 address or a domain name).
     * \param port Port to connect to (service names are not accepted by
     *        this interface, only port numbers are accepted).
     * \param family Optionally, the address family to be used. Use
     *        "PF_INET" to indicate IPv4, "PF_INET6" to indicate IPv6,
     *        "PF_UNSPEC" to prefer IPv4 addresses to IPv6 addresses,
     *        and "PF_UNSPEC6" to prefer IPv6 to IPv4.
     */
    Stream(std::string address, std::string port,
        std::string family = "PF_UNSPEC");

    /*!
     * \brief Create a stream connecting to the specified remote host.
     * \remark This is syntactic sugar for Stream::Stream().
     * \see Stream::Stream().
     */
    Stream connect(std::string address, std::string port,
            std::string family = "PF_UNSPEC") {
        return Stream(address, port, family);
    }

    /*!
     * \brief Create a stream attached to an already connected socket.
     * \sock The already connecte socket.
     */
    Stream(evutil_socket_t sock);

    /*!
     * \brief Register callback for "connect" event.
     * \param fn Callback invoked when the connection is established.
     * \remark The callback is not called if you constructed this
     *         object attaching it to an already opened socket.
     */
    void on_connect(std::function<void(void)>&& fn);

    /*!
     * \brief Destructor.
     */
    ~Stream(void);

    //
    // Low-level interface for sending data to the other end:
    //

    /*!
     * \brief Write data on the underlying socket.
     * \param data Data to be sent on the underlying socket.
     * \throws std::runtime_error on error.
     * \returns A reference to this stream for chaining operations.
     */
    Stream& operator<<(std::string data);

    /*!
     * \brief Register callback for "flush" event.
     * \param fn Callback invoked when the underlying socket is flushed.
     * \remark This callback allows you to control when to send more data
     *         after some data was already passed to the kernel.
     */
    void on_flush(std::function<void(void)>&& fn);

    //
    // Events generated by the HTTP response parser:
    //

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
      std::map<std::string, std::string>&& headers)>&& fn);

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
};

class Response {
public:

    unsigned int status_code;
    std::string reason;
    unsigned short http_major;
    unsigned short http_minor;

    std::string response_line;

    Headers headers;
    //body.read()
    Body body;
}

class Client {
    Stream *stream = nullptr;

    public:

    Client(Client&) = delete;
    Client& operator=(Client&) = delete;

    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

    Client(void) {

    }

    Client(DNSResolver resolver) {

    }

    // XXX Add proxy support.

    /*!
     * \brief Perform a HTTP Request.
     * \param headers The HTTP headers to use in the request
     * \param request_options A std::map with key values of the options
     *                        supported.
     *                        {
     *                          "follow_redirects": "yes|no",
     *                          "url": std::string,
     *                          "ignore_body": "yes|no",
     *                          "method": "GET|DELETE|PUT|POST|HEAD ...",
     *                          "http_version": "HTTP/1.1",
     *                          "path": by default is taken from the url
     *                        }
     *  \param body_callback A callback that is fired every time a new chunk of
     *                       the body is received.
     *
     */
    Response request(Options request_options);

    Response request(Options request_options,
            Headers headers);

    Response request(Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    /*!
     * \brief Perform HTTP GET request.
     * \param url  The url of the request.
     * \param body The `body` of the request.
     * \param request_options A std::map with key values of the options
     *                        supported.
     * \param headers The HTTP headers to use in the request
     * \param body_callback A callback that is fired every time a new chunk of
     *                      the body is received.
     */
    Response get(std::string url,
            std::string body,
            Options request_options,
            Headers headers);

    Response get(std::string url,
            std::string body,
            Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    /*!
     * \brief Perform HTTP POST request.
     * \param url  The url of the request.
     * \param body The `body` of the request.
     * \param request_options A std::map with key values of the options
     *                        supported.
     * \param headers The HTTP headers to use in the request
     * \param body_callback A callback that is fired every time a new chunk of
     *                      the body is received.
     */
    Response post(std::string url,
            std::string body,
            Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    Response post(std::string url,
            std::string body,
            Options request_options,
            Headers headers);

    /*!
     * \brief Perform HTTP PUT request.
     * \param url  The url of the request.
     * \param body The `body` of the request.
     * \param request_options A std::map with key values of the options
     *                        supported.
     * \param headers The HTTP headers to use in the request
     * \param body_callback A callback that is fired every time a new chunk of
     *                      the body is received.
     */
    Response put(std::string url,
            std::string body,
            Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    Response put(std::string url,
            std::string body,
            Options request_options,
            Headers headers);

    /*!
     * \brief Perform HTTP HEAD request.
     * \param url  The url of the request.
     * \param body The `body` of the request.
     * \param request_options A std::map with key values of the options
     *                        supported.
     * \param headers The HTTP headers to use in the request
     * \param body_callback A callback that is fired every time a new chunk of
     *                      the body is received.
     */
    Response head(std::string url,
            std::string body,
            Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    Response head(std::string url,
            std::string body,
            Options request_options,
            Headers headers);

    /*!
     * \brief Perform HTTP DELETE request.
     * \param url  The url of the request.
     * \param body The `body` of the request.
     * \param request_options A std::map with key values of the options
     *                        supported.
     * \param headers The HTTP headers to use in the request
     * \param body_callback A callback that is fired every time a new chunk of
     *                      the body is received.
     */
    Response del(std::string url,
            std::string body,
            Options request_options,
            Headers headers,
            std::function<void(std::string&&)>&& body_callback);

    Response del(std::string url,
            std::string body,
            Options request_options,
            Headers headers);

}

}}  // namespaces
#endif  // LIBIGHT_NET_HTTP_HPP
