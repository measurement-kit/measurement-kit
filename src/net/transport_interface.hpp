// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_TRANSPORT_INTERFACE_HPP
#define SRC_NET_TRANSPORT_INTERFACE_HPP

#include <functional>
#include <measurement_kit/net/buffer.hpp>
#include <stddef.h>
#include <string>

namespace mk {

// Forward declaration
class Error;

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

} // namespace net
} // namespace mk
#endif
