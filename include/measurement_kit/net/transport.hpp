// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

#include <measurement_kit/dns.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <stddef.h>

// Forward declaration
struct bufferevent;

namespace mk {
namespace net {

class Transport {
  public:
    virtual void emit_connect() = 0;
    virtual void emit_data(Buffer buf) = 0;
    virtual void emit_flush() = 0;
    virtual void emit_error(Error err) = 0;

    virtual ~Transport() {}

    virtual void on_connect(std::function<void()>) = 0;
    virtual void on_data(std::function<void(Buffer)>) = 0;
    virtual void on_flush(std::function<void()>) = 0;
    virtual void on_error(std::function<void(Error)>) = 0;

    virtual void record_received_data() = 0;
    virtual void dont_record_received_data() = 0;
    virtual Buffer &received_data() = 0;

    virtual void record_sent_data() = 0;
    virtual void dont_record_sent_data() = 0;
    virtual Buffer &sent_data() = 0;

    virtual void set_timeout(double) = 0;
    virtual void clear_timeout() = 0;

    virtual void write(const void *, size_t) = 0;
    virtual void write(std::string) = 0;
    virtual void write(Buffer) = 0;

    virtual void close(std::function<void()>) = 0;

    virtual std::string socks5_address() = 0;
    virtual std::string socks5_port() = 0;
};

/*
 *  Syntactic sugar when you need only to write or read (vis a vis Transport,
 *  required when you need read and write at the same time).
 */

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb);

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb,
           Var<Reactor> reactor = Reactor::global());

inline void read(Var<Transport> t, Var<Buffer> buff, Callback<Error> callback,
                 Var<Reactor> reactor = Reactor::global()) {
    readn(t, buff, 1, callback, reactor);
}

} // namespace net
} // namespace mk
#endif
