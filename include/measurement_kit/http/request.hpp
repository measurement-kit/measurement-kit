// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_REQUEST_HPP
#define MEASUREMENT_KIT_HTTP_REQUEST_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/error.hpp>

#include <measurement_kit/http/headers.hpp>
#include <measurement_kit/http/request_serializer.hpp>
#include <measurement_kit/http/response.hpp>
#include <measurement_kit/http/response_parser.hpp>
#include <measurement_kit/http/stream.hpp>

#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>

namespace measurement_kit {
namespace http {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

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
    Logger *logger = Logger::global();

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
            Logger *lp = Logger::global(),
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
            if (err != EOFError()) {
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
            *stream << buf.read();

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

} // namespace http 
} // namespace measurement_kit
#endif
