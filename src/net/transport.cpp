// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/net.hpp>
#include "src/net/connect.hpp"
#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include "src/net/socks5.hpp"

namespace mk {
namespace net {

void connect(std::string address, int port,
             std::function<void(Error, Var<Transport>)> callback,
             Settings settings, Logger *logger, Poller *poller) {
    if (settings.find("dumb_transport") != settings.end()) {
        callback(NoError(), Var<Transport>(new Emitter(logger)));
        return;
    }
    if (settings.find("socks5_proxy") != settings.end()) {
        socks5_connect(address, port, settings, callback, poller, logger);
        return;
    }
    double timeout = settings.get("timeout", 30.0);
    net::connect(address, port, [=](ConnectResult r) {
        // TODO: it would be nice to pass to this callback a compound error
        // that also contains info on all what went wrong when connecting
        if (r.overall_error) {
            callback(r.overall_error, nullptr);
            return;
        }
        Var<Transport> txp = Connection::make(r.connected_bev, poller, logger);
        txp->set_timeout(timeout);
        callback(NoError(), txp);
    }, timeout, poller, logger);
}

} // namespace net
} // namespace mk
