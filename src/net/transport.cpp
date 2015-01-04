/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include "net/connection.hpp"
#include "net/socks5.hpp"
#include "net/transport.hpp"

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
        // Use Tor default settings: localhost and 9050
        if (settings.find("socks5_address") == settings.end()) {
            if (settings["family"] != "PF_INET6") {
                settings["socks5_address"] = "127.0.0.1";
            } else {
                settings["socks5_address"] = "::1";
            }
        }
        if (settings.find("socks5_port") == settings.end()) {
            settings["socks5_port"] = "9050";
        }
        return std::make_shared<Socks5>(settings);
    }

    return std::make_shared<Connection>(settings["family"].c_str(),
            settings["address"].c_str(), settings["port"].c_str());
}

}}}
