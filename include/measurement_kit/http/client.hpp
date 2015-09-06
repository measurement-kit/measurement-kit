// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_CLIENT_HPP
#define MEASUREMENT_KIT_HTTP_CLIENT_HPP

#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/logger.hpp>

#include <measurement_kit/http/request.hpp>
#include <measurement_kit/http/response_parser.hpp>

#include <iosfwd>
#include <set>
#include <string>
#include <type_traits>

namespace measurement_kit {
namespace http {

using namespace measurement_kit::common;
using namespace measurement_kit::net;

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
            Logger *lp = Logger::global()) {
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

} // namespace http
} // namespace measurement_kit
#endif
