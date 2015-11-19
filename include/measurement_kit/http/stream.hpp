// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_STREAM_HPP
#define MEASUREMENT_KIT_HTTP_STREAM_HPP

#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/var.hpp>

#include <measurement_kit/net/error.hpp>
#include <measurement_kit/net/transport.hpp>

#include <measurement_kit/http/response_parser.hpp>

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <type_traits>

namespace measurement_kit {
namespace net {
class Buffer;
}
namespace http {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

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
    Transport connection;
    Var<ResponseParser> parser;
    std::function<void(Error)> error_handler;
    std::function<void()> connect_handler;

    void connection_ready(void) {
        connection.on_data([&](Buffer &data) {
            parser->feed(data);
        });
        //
        // Intercept EOF error to implement body-ends-at-EOF semantic.
        //
        connection.on_error([&](Error error) {
            auto safe_eh = error_handler;
            if (error == EOFError()) {
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
    Var<ResponseParser> get_parser() {
        return parser;
    }

    Transport get_transport() {
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
    Stream(Settings settings, Logger *lp = Logger::global()) {
        parser = std::make_shared<ResponseParser>(lp);
        connection = connect(settings, lp);
        //
        // While the connection is in progress, just forward the
        // error if needed, we'll deal with body-terminated-by-EOF
        // semantic when we know we are actually connected.
        //
        connection.on_error([&](Error error) {
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
        connection.on_connect([this]() {
            connection_ready();
        });
    }

    /*!
     * \brief Close this stream.
     */
    void close() {
        connection.close();
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
        connection.send(data);
        return *this;
    }

    /*!
     * \brief Register callback for "flush" event.
     * \param fn Callback invoked when the underlying socket is flushed.
     * \remark This callback allows you to control when to send more data
     *         after some data was already passed to the kernel.
     */
    void on_flush(std::function<void(void)>&& fn) {
        connection.on_flush(std::move(fn));
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
        connection.set_timeout(timeo);
    }

    std::string socks5_address() {
        return connection.socks5_address();
    }

    std::string socks5_port() {
        return connection.socks5_port();
    }
};

}}
#endif
