// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/error.hpp>
#include <string>
#include "src/net/bev-protocols.hpp"
#include "src/net/bev-transport.hpp"

namespace mk {
namespace net {

void http_09_sendrecv(Var<TransportBev> tbev, std::string request,
                      std::function<void(Error)> callback,
                      std::string *response, double timeout) {
    tbev->send(request);
    if (timeout >= 0.0) {
        tbev->set_timeout(timeout);
    }
    tbev->on_data([response](Buffer data) {
        if (response) *response += data.read();
    });
    tbev->on_error([callback, tbev](Error err) {
        tbev->on_error(nullptr);  // Lose reference to tbev
        if (err == EOFError()) {
            err = NoError();
        }
        callback(err);
    });
}

} // namespace net
} // namespace mk
