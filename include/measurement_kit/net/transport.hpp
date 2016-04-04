// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

#include <functional>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <stddef.h>
#include <string>
#include <vector>

// Forward declaration
struct bufferevent;

namespace mk {

// Forward declaration
class Error;

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

struct ResolveHostnameResult {
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;

    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;

    std::vector<std::string> addresses;
};

struct ConnectResult : public ErrorContext {
    ResolveHostnameResult resolve_result;
    std::vector<Error> connect_result;
    bufferevent *connected_bev = nullptr;
};

void connect(std::string address, int port,
             Callback<Var<Transport>> callback,
             Settings settings = {},
             Logger *logger = Logger::global(),
             Poller *poller = Poller::global());

using ConnectManyCb = Callback<std::vector<Var<Transport>>>;

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings = {},
        Logger *logger = Logger::global(),
        Poller *poller = Poller::global());

// TODO: these functions should be in src/net/transport.cpp

// Asynchronous write() of the content of the buffer
inline void write(Var<Transport> txp, Buffer buf, Callback<> callback) {
    txp->on_flush([=]() {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        callback(NoError());
    });
    txp->on_error([=](Error err) {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        callback(err);
    });
    txp->write(buf);
}

// Like read() but only callback when N bytes have been recv'd
inline void readn(Var<Transport> txp, Var<Buffer> buff, size_t n,
                  Callback<> callback) {
    if (buff->length() >= n) {
        callback(NoError());
        return;
    }
    txp->on_data([=](Buffer d) {
        *buff << d;
        if (buff->length() < n) {
            return;
        }
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        callback(NoError());
    });
    txp->on_error([=](Error error) {
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        callback(error);
    });
}

// Asynchronous read of data to be stored into the buffer
inline void read(Var<Transport> t, Var<Buffer> buff, Callback<> callback) {
    readn(t, buff, 1, callback);
}

} // namespace net
} // namespace mk
#endif
