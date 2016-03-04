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
#include <measurement_kit/common/maybe.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>

#include <measurement_kit/net/buffer.hpp>

namespace mk {
namespace net {

class TransportInterface {
  public:
    virtual void emit_connect() = 0;

    virtual void emit_data(Buffer) = 0;

    virtual void emit_flush() = 0;

    virtual void emit_error(Error) = 0;

    TransportInterface() {}

    virtual ~TransportInterface() {}

    virtual void on_connect(std::function<void()>) = 0;

    virtual void on_data(std::function<void(Buffer)>) = 0;

    virtual void on_flush(std::function<void()>) = 0;

    virtual void on_error(std::function<void(Error)>) = 0;

    virtual void set_timeout(double) = 0;

    virtual void clear_timeout() = 0;

    virtual void send(const void *, size_t) = 0;

    virtual void send(std::string) = 0;

    virtual void send(Buffer) = 0;

    virtual void close() = 0;

    virtual std::string socks5_address() = 0;

    virtual std::string socks5_port() = 0;
};

/// A connection with TCP-like properties
class Transport {
  public:

    /// Emit the CONNECT event
    void emit_connect() const { impl->emit_connect(); };

    /// Emit the DATA event
    void emit_data(Buffer buf) const { impl->emit_data(buf); };

    /// Emit the FLUSH event
    void emit_flush() const { impl->emit_flush(); };

    /// Emit the ERROR event
    void emit_error(Error err) const { impl->emit_error(err); };

    /// Construct from pointer to interface
    Transport(TransportInterface *ptr = nullptr) : impl(ptr) {}

    ~Transport() {}; ///< Destructor

    /// Set CONNECT handler
    void on_connect(std::function<void()> f) const { impl->on_connect(f); }

    /// Set DATA handler
    void on_data(std::function<void(Buffer)> f) const { impl->on_data(f); }

    /// Set FLUSH handler
    void on_flush(std::function<void()> f) const { impl->on_flush(f); }

    /// Set ERROR handler
    void on_error(std::function<void(Error)> f) const { impl->on_error(f); }

    /// Set I/O timeout
    void set_timeout(double t) const { impl->set_timeout(t); }

    /// Clear I/O timeout
    void clear_timeout() const { impl->clear_timeout(); }

    /// Send N bytes of data starting from P
    void send(const void *p, size_t n) const { impl->send(p, n); }

    /// Send data contained by string
    void send(std::string d) const { impl->send(d); }

    /// Send data contained by buffer (this method is zero copy)
    void send(Buffer d) const { impl->send(d); }

    /// Close the underlying socket
    void close() const { impl->close(); }

    /// Get SOCKS5 proxy address (if any)
    std::string socks5_address() const { return impl->socks5_address(); }

    /// Get SOCKS5 proxy port (if any)
    std::string socks5_port() const { return impl->socks5_port(); }

  private:
    Var<TransportInterface> impl;
};

/// Connects a transport
Maybe<Transport> connect(Settings, Logger * = Logger::global(),
                         Poller * = Poller::global());

/// Connects a transport (alternative syntax)
void connect(std::string address, int port,
             std::function<void(Error, Transport)> callback,
             Settings settings = {},
             Logger *logger = Logger::global(),
             Poller *poller = Poller::global());

} // namespace net
} // namespace mk
#endif
