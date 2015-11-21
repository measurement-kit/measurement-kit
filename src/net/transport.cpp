// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/connection.hpp"
#include "src/net/dumb.hpp"
#include "src/net/socks5.hpp"
#include <measurement_kit/net/transport.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

static Transport connect_internal(Settings settings, Logger *logger) {

    if (settings.find("dumb_transport") != settings.end()) {
        return Transport(new Dumb(logger));
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
        return Transport(new Socks5(settings, logger));
    }

    return Transport(new Connection(settings["family"].c_str(),
                     settings["address"].c_str(), settings["port"].c_str(),
                     logger));
}

Transport connect(Settings settings, Logger *lp) {
    double timeo = 30.0;
    if (settings.find("timeout") != settings.end()) {
        size_t invalid;
        timeo = std::stod(settings["timeout"], &invalid);
        if (invalid != settings["timeout"].length()) {
            throw std::runtime_error("invalid argument");
        }
    }
    auto transport = connect_internal(settings, lp);
    if (timeo >= 0.0) {
        transport.set_timeout(timeo);
    }
    return transport;
}

}}
