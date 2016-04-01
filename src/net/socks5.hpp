// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_SOCKS5_HPP
#define SRC_NET_SOCKS5_HPP

#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class Socks5 : public Emitter {
  public:
    // Constructor that attaches to already existing transport
    Socks5(Var<Transport>, Settings, Poller * = Poller::global(),
            Logger * = Logger::global());

    // Constructor that creates its new own transport
    Socks5(Settings, Logger * = Logger::global(), Poller * = Poller::global());

    void set_timeout(double timeout) override { conn->set_timeout(timeout); }

    void clear_timeout() override { conn->clear_timeout(); }

    void send(Buffer data) override { conn->send(data); }

    void close() override {
        isclosed = true;
        conn->close();
    }

    std::string socks5_address() override { return proxy_address; }

    std::string socks5_port() override { return proxy_port; }

  protected:
    void socks5_connect_();

    Settings settings;
    Var<Transport> conn;
    Buffer buffer;
    bool isclosed = false;
    std::string proxy_address;
    std::string proxy_port;
};

void socks5_connect(std::string address, int port, Settings settings,
        std::function<void(Error, Var<Transport>)> callback,
        Poller *poller = Poller::global(), Logger *logger = Logger::global());

} // namespace net
} // namespace mk
#endif
