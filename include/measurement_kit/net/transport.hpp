// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

//
// Generic transport (stream socket, SOCKSv5, etc.)
//

#include <functional>
#include <string>

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>

#include <measurement_kit/net/buffer.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

class TransportInterface {
  public:

    virtual void emit_connect() = 0;

    virtual void emit_data(Buffer &) = 0;

    virtual void emit_flush() = 0;

    virtual void emit_error(Error) = 0;

    TransportInterface() {}

    virtual ~TransportInterface() {}

    virtual void on_connect(std::function<void()>) = 0;

    virtual void on_ssl(std::function<void()>) = 0;

    virtual void on_data(std::function<void(Buffer &)>) = 0;

    virtual void on_flush(std::function<void()>) = 0;

    virtual void on_error(std::function<void(Error)>) = 0;

    virtual void set_timeout(double) = 0;

    virtual void clear_timeout() = 0;

    virtual void send(const void*, size_t) = 0;

    virtual void send(std::string) = 0;

    virtual void send(Buffer &) = 0;

    virtual void close() = 0;

    virtual std::string socks5_address() = 0;

    virtual std::string socks5_port() = 0;
};

class Transport : public TransportInterface {
  public:

    void emit_connect() override { t->emit_connect(); }

    virtual void emit_data(Buffer &d) override { t->emit_data(d); }

    virtual void emit_flush() override { t->emit_flush(); }

    virtual void emit_error(Error e) override { t->emit_error(e); }

    Transport() {}

    Transport(TransportInterface *p) { t.reset(p); }

    ~Transport() override {}

    void on_connect(std::function<void()> f) override { t->on_connect(f); }

    void on_ssl(std::function<void()> f) override { t->on_ssl(f); }

    void on_data(std::function<void(Buffer &)> f) override { t->on_data(f); }

    void on_flush(std::function<void()> f) override { t->on_flush(f); }

    void on_error(std::function<void(Error)> f) override { t->on_error(f); }

    void set_timeout(double d) override { t->set_timeout(d); }

    void clear_timeout() override { t->clear_timeout(); }

    void send(const void *p, size_t n) override { t->send(p, n); }

    void send(std::string s) override { t->send(s); }

    Transport &operator<<(std::string s) {
        send(s);
        return *this;
    }

    void send(Buffer &d) override { t->send(d); }

    Transport &operator<<(Buffer &d) {
        send(d);
        return *this;
    }

    void close() override { t->close(); }

    std::string socks5_address() override { return t->socks5_address(); }

    std::string socks5_port() override { return t->socks5_port(); }

  private:
    Var<TransportInterface> t;
};

Transport connect(Settings, Logger * = Logger::global());

} // namespace net
} // namespace measurement_kit
#endif
