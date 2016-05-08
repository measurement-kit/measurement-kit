// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/http.hpp>
#include "src/http/request.hpp"

namespace mk {
namespace http {

// TODO: This function should be probably be moved in src/http/request.cpp
void request(Settings settings, RequestCallback cb, Headers headers,
             std::string body, Var<Logger> lp, Poller *po) {
    request_cycle(
        settings, headers, body, [cb](Error err, Var<Response> re) {
            // TODO: maybe change RequestCallback to receive a Var<Response>?
            Response rex;
            if (re) {
                rex = *re;
            }
            cb(err, rex);
        }, po, lp);
}

} // namespace http
} // namespace mk
