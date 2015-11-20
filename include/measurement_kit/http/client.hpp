// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_CLIENT_HPP
#define MEASUREMENT_KIT_HTTP_CLIENT_HPP

#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/http/headers.hpp>
#include <measurement_kit/http/response.hpp>

#include <iosfwd>
#include <set>
#include <string>
#include <type_traits>

namespace measurement_kit {
namespace http {

using namespace measurement_kit::common;

class Request;

typedef std::function<void(Error, Response)> RequestCallback;

class Client {

protected:
    std::set<Request *> pending;

public:
    /*!
     * \brief Issue HTTP request.
     * \see Request::Request.
     */
    void request(Settings settings, Headers headers,
            std::string body, RequestCallback callback,
            Logger *lp = Logger::global());

    Client() {
        // nothing to do
    }

    ~Client();

    //
    // TODO: implement all the fancy methods
    //
};

} // namespace http
} // namespace measurement_kit
#endif
