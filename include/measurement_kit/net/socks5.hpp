// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_SOCKS5_HPP
#define MEASUREMENT_KIT_NET_SOCKS5_HPP

#include <measurement_kit/common/logger.hpp>

#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/connection.hpp>
#include <measurement_kit/net/dumb.hpp>
#include <measurement_kit/net/transport.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

class Socks5 : public Dumb {
protected:
    SharedPointer<Connection> conn;
    Settings settings;
    SharedPointer<Buffer> buffer{
        std::make_shared<Buffer>()
    };
    bool isclosed = false;
    std::string proxy_address;
    std::string proxy_port;

public:
    Socks5(Settings, Logger *lp = Logger::global());

    void set_timeout(double timeout) override {
        conn->set_timeout(timeout);
    }

    void clear_timeout() override {
        conn->clear_timeout();
    }

    void send(const void* data, size_t count) override {
        conn->send(data, count);
    }

    void send(std::string data) override {
        conn->send(data);
    }

    void send(Buffer& data) override {
        conn->send(data);
    }

    void close() override {
        isclosed = true;
        conn->close();
    }

    std::string socks5_address() override {
        return proxy_address;
    }

    std::string socks5_port() override {
        return proxy_port;
    }
};

}}
#endif
