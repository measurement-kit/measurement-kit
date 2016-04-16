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

static Var<Transport> connect_internal(Settings settings, Logger *logger,
                                       Poller *poller) {

    if (settings.find("dumb_transport") != settings.end()) {
        return Var<Transport>(new Emitter(logger));
    }

    if (settings.find("family") == settings.end()) {
        settings["family"] = "PF_UNSPEC";
    }

    if (settings.find("address") == settings.end()) {
        throw std::runtime_error("invalid argument");
    }
    if (settings.find("port") == settings.end()) {
        throw std::runtime_error("invalid argument");
    }

    if (settings.find("socks5_proxy") != settings.end()) {
        auto proxy = settings["socks5_proxy"];
        auto pos = proxy.find(":");
        if (pos == std::string::npos) {
            throw std::runtime_error("invalid argument");
        }
        auto address = proxy.substr(0, pos);
        auto port = proxy.substr(pos + 1);
        settings["socks5_address"] = address;
        settings["socks5_port"] = port;
        return Var<Transport>(new Socks5(settings, logger, poller));
    }

    return Var<Transport>(new Connection(settings["family"].c_str(),
                          settings["address"].c_str(),
                          settings["port"].c_str(),
                          logger, poller));
}

ErrorOr<Var<Transport>> connect(Settings settings, Logger *lp, Poller *poller) {
    double timeo = 30.0;
    if (settings.find("timeout") != settings.end()) {
        timeo = settings["timeout"].as<double>();
    }
    Var<Transport> transport = connect_internal(settings, lp, poller);
    if (timeo >= 0.0) {
        transport->set_timeout(timeo);
    }
    return transport;
}

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
        Var<Transport> txp(new Connection(r.connected_bev, poller, logger));
        txp->set_timeout(timeout);
        callback(NoError(), txp);
    }, timeout, poller, logger);
}

} // namespace net
} // namespace mk
