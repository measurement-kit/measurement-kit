// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_SOCKS5_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_SOCKS5_HPP

#include "src/libmeasurement_kit/net/emitter.hpp"

namespace mk {
namespace net {

/*
 * TODO: now that events are not emitted after close, we can safely
 * refactor this as a factory function, because we do not need to
 * worry about whether the `on_connect` callback has invoked `close`
 * before emitting `DATA` with extra data that we may have read.
 */
class Socks5 : public Emitter {
  public:
    // Constructor that attaches to already existing transport
    Socks5(SharedPtr<Transport>, Settings, SharedPtr<Reactor>, SharedPtr<Logger>);

    void set_timeout(double timeout) override { conn->set_timeout(timeout); }

    void clear_timeout() override { conn->clear_timeout(); }

  protected:
    void adjust_timeout(double) override { /* NOTHING */ }

    void start_writing() override { conn->write(output_buff); }

  public:
    void close(std::function<void()> callback) override {
        isclosed = true;
        conn->close(callback);
    }

    std::string socks5_address() override { return proxy_address; }

    std::string socks5_port() override { return proxy_port; }

  protected:
    void socks5_connect_();

    Settings settings;
    SharedPtr<Transport> conn;
    Buffer buffer;
    bool isclosed = false;
    std::string proxy_address;
    std::string proxy_port;
};

Buffer socks5_format_auth_request(SharedPtr<Logger> = Logger::global());
ErrorOr<bool> socks5_parse_auth_response(Buffer &, SharedPtr<Logger> = Logger::global());
ErrorOr<Buffer> socks5_format_connect_request(Settings, SharedPtr<Logger> = Logger::global());
ErrorOr<bool> socks5_parse_connect_response(Buffer &, SharedPtr<Logger> = Logger::global());

void socks5_connect(std::string address, int port, Settings settings,
        std::function<void(Error, SharedPtr<Transport>)> callback,
        SharedPtr<Reactor> reactor = Reactor::global(), SharedPtr<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
