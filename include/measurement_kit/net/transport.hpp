// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

#include <functional>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <stddef.h>
#include <string>

namespace mk {

// Forward declaration
class Error;

namespace net {

// Forward declaration
class TransportInterface;

/// A connection with TCP-like properties
class Transport {
  public:
    void emit_connect() const;        ///< Emit CONNECT event
    void emit_data(Buffer buf) const; ///< Emit DATA event
    void emit_flush() const;          ///< Emit FLUSH event
    void emit_error(Error err) const; ///< Emit ERROR event

    Transport(TransportInterface * = nullptr); ///< Constructor
    ~Transport() {};                           ///< Destructor

    void on_connect(std::function<void()>) const;    ///< Set CONNECT handler
    void on_data(std::function<void(Buffer)>) const; ///< Set DATA handler
    void on_flush(std::function<void()>) const;      ///< Set FLUSH handler
    void on_error(std::function<void(Error)>) const; ///< Set ERROR handler

    void set_timeout(double) const; ///< Set I/O timeout
    void clear_timeout() const;     ///< Clear I/O timeout

    void send(const void *, size_t) const; ///< Send C string
    void send(std::string) const;          ///< Send C++ string
    void send(Buffer) const;               ///< Send buffer

    void close() const; ///< Close underlying socket

    std::string socks5_address() const; ///< Get SOCKS5 proxy address (if any)
    std::string socks5_port() const;    ///< Get SOCKS5 proxy port (if any)

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
