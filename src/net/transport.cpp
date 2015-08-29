// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/net/connection.hpp>
#include <measurement_kit/net/dumb.hpp>
#include <measurement_kit/net/socks5.hpp>
#include <measurement_kit/net/transport.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

static SharedPointer<Transport>
connect_internal(Settings settings, Logger *logger) {

    if (settings.find("dumb_transport") != settings.end()) {
        return std::make_shared<Dumb>(logger);
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
        return std::make_shared<Socks5>(settings, logger);
    }

    return std::make_shared<Connection>(settings["family"].c_str(),
            settings["address"].c_str(), settings["port"].c_str(),
            logger);
}

SharedPointer<Transport> connect(Settings settings, Logger *lp) {
    double timeo = -1.0;  // No timeout by default
    if (settings.find("timeout") != settings.end()) {
        size_t invalid;
        timeo = std::stod(settings["timeout"], &invalid);
        if (invalid != settings["timeout"].length()) {
            throw std::runtime_error("invalid argument");
        }
    }
    auto transport = connect_internal(settings, lp);
    if (timeo >= 0.0) {
        transport->set_timeout(timeo);
    }
    return transport;
}

}}
