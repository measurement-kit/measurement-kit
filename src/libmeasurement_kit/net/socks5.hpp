// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_SOCKS5_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_SOCKS5_HPP

#include "../net/emitter.hpp"

#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

/*
 * TODO: to continue refactoring here we should probably inherit from
 * emitter-base rather than emitter. I am not doing it as part of this
 * diff, because that is more than hotfix refactoring.
 */
class Socks5 : public Emitter {
  public:
    // Constructor that attaches to already existing transport
    Socks5(Var<Transport>, Settings, Var<Reactor>, Var<Logger>);

    void set_timeout(double timeout) override { conn->set_timeout(timeout); }

    void clear_timeout() override { conn->clear_timeout(); }

    void start_writing() override { conn->write(output_buff); }

    void close(std::function<void()> callback) override {
        isclosed = true;
        conn->close(callback);
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

Buffer socks5_format_auth_request(Var<Logger> = Logger::global());
ErrorOr<bool> socks5_parse_auth_response(Buffer &, Var<Logger> = Logger::global());
ErrorOr<Buffer> socks5_format_connect_request(Settings, Var<Logger> = Logger::global());
ErrorOr<bool> socks5_parse_connect_response(Buffer &, Var<Logger> = Logger::global());

void socks5_connect(std::string address, int port, Settings settings,
        std::function<void(Error, Var<Transport>)> callback,
        Var<Reactor> reactor = Reactor::global(), Var<Logger> logger = Logger::global());

} // namespace net
} // namespace mk
#endif
