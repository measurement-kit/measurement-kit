/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <ight/net/connection.hpp>
#include <ight/net/socks5.hpp>
#include <ight/net/transport.hpp>

namespace ight {
namespace net {
namespace transport {

using namespace ight::common;
using namespace ight::common::pointer;

using namespace ight::net::connection;
using namespace ight::net::socks5;

SharedPointer<Transport> connect(Settings settings) {

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
        return std::make_shared<Socks5>(settings);
    }

    return std::make_shared<Connection>(settings["family"].c_str(),
            settings["address"].c_str(), settings["port"].c_str());
}

}}}
