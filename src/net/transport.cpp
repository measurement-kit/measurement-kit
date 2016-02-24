// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include "src/net/socks5.hpp"
#include <measurement_kit/net/transport.hpp>

namespace mk {
namespace net {

static Transport connect_internal(Settings settings, Logger *logger,
                                       Poller *poller) {

    if (settings.find("dumb_transport") != settings.end()) {
        return Transport(new Emitter(logger));
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
        return Transport(new Socks5(settings, logger, poller));
    }

    return Transport(new Connection(settings["family"].c_str(),
                                         settings["address"].c_str(),
                                         settings["port"].c_str(),
                                         logger, poller));
}

Maybe<Transport> connect(Settings settings, Logger *lp, Poller *poller) {
    double timeo = 30.0;
    if (settings.find("timeout") != settings.end()) {
        size_t invalid;
        timeo = std::stod(settings["timeout"], &invalid);
        if (invalid != settings["timeout"].length()) {
            throw std::runtime_error("invalid argument");
        }
    }
    Transport transport = connect_internal(settings, lp, poller);
    if (timeo >= 0.0) {
        transport.set_timeout(timeo);
    }
    return Maybe<Transport>(transport);
}

void connect(std::string address, int port,
             std::function<void(Error, Transport)> callback,
             Settings settings, Logger *logger, Poller *poller) {
    settings["address"] = address;
    settings["port"] = std::to_string(port);
    Maybe<Transport> maybe = connect(settings, logger, poller);
    if (!maybe) {
        callback(maybe.as_error(), Transport{});
        return;
    }
    auto transport = maybe.as_value();
    transport.on_connect([callback, transport]() {
        transport.on_connect(nullptr);
        transport.on_error(nullptr);
        callback(NoError(), transport);
    });
    transport.on_error([callback, transport](Error error) {
        transport.on_connect(nullptr);
        transport.on_error(nullptr);
        callback(error, transport);
    });
}

} // namespace net
} // namespace mk
