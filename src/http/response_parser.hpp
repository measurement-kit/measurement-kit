// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP
#define MEASUREMENT_KIT_HTTP_RESPONSE_PARSER_HPP

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/http/error.hpp>

#include <functional>
#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace measurement_kit {
namespace net {
class Buffer;
}
namespace http {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

class ResponseParserImpl; // See http.cpp

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
 *     auto parser = measurement_kit::http::ResponseParser();
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
 *     auto connection = measurement_kit::net::connect(...);
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
    ResponseParser(Logger * = Logger::global());

    /*!
     * \brief Deleted copy constructor.
     */
    ResponseParser(ResponseParser &other) = delete;

    /*!
     * \brief Deleted copy assignment.
     */
    ResponseParser &operator=(ResponseParser &other) = delete;

    /*!
     * \brief Move constructor.
     */
    ResponseParser(ResponseParser &&other) { std::swap(impl, other.impl); }

    /*!
     * \brief Move assignment.
     */
    ResponseParser &operator=(ResponseParser &&other) {
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
    void on_begin(std::function<void(void)> &&fn);

    /*!
     * \brief Register `headers_complete` event handler.
     * \param fn The `headers_complete` event handler.
     * \remark The parameters received by the event handler are, respectively,
     *         the HTTP major version number, the HTTP minor version number,
     *         the status code, the reason string, and a map containing the
     *         HTTP headers.
     */
    void on_headers_complete(
        std::function<void(unsigned short http_major, unsigned short http_minor,
                           unsigned int status_code, std::string &&reason,
                           Headers &&headers)> &&fn);

    /*!
     * \brief Register `body` event handler.
     * \param fn The `body` event handler.
     * \remark The parameter received by the event handler is the received
     *         piece of body.
     * \remark By default this event handler is unset, meaning that the
     *         received body is ignored.
     */
    void on_body(std::function<void(std::string &&)> &&fn);

    /*!
     * \brief Register `end` event handler.
     * \param fn The `end` event handler.
     */
    void on_end(std::function<void(void)> &&fn);

    /*!
     * \brief Feed the parser.
     * \param data Evbuffer containing the received data.
     * \throws std::runtime_error This method throws std::runtime_error (or
     *         a class derived from it) on several error conditions.
     */
    void feed(Buffer &data);

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

} // namespace http
} // namespace measurement_kit
#endif
