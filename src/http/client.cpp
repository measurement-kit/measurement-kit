// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/http/client.hpp>
#include "src/http/request.hpp"

namespace measurement_kit {
namespace http {

void Client::request(Settings settings, Headers headers,
        std::string body, RequestCallback callback,
        Logger *lp) {
    auto r = new Request(settings, headers, body,
            std::move(callback), lp, &pending);
    pending.insert(r);
}

Client::~Client() {
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

} // namespace http
} // namespace measurement_kit
