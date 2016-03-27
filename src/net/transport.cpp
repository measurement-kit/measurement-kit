// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/net.hpp>
#include "src/net/connect.hpp"
#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include "src/net/transport_interface.hpp"
#include "src/net/socks5.hpp"

namespace mk {
namespace net {

void Transport::emit_connect() const { impl->emit_connect(); };

void Transport::emit_data(Buffer buf) const { impl->emit_data(buf); };

void Transport::emit_flush() const { impl->emit_flush(); };

void Transport::emit_error(Error err) const { impl->emit_error(err); };

Transport::Transport(TransportInterface *ptr) : impl(ptr) {}

void Transport::on_connect(std::function<void()> f) const {
    impl->on_connect(f);
}

void Transport::on_data(std::function<void(Buffer)> f) const {
    impl->on_data(f);
}

void Transport::on_flush(std::function<void()> f) const { impl->on_flush(f); }

void Transport::on_error(std::function<void(Error)> f) const {
    impl->on_error(f);
}

void Transport::set_timeout(double t) const { impl->set_timeout(t); }

void Transport::clear_timeout() const { impl->clear_timeout(); }

void Transport::send(const void *p, size_t n) const { impl->send(p, n); }

void Transport::send(std::string d) const { impl->send(d); }

void Transport::send(Buffer d) const { impl->send(d); }

void Transport::close() const { impl->close(); }

std::string Transport::socks5_address() const { return impl->socks5_address(); }

std::string Transport::socks5_port() const { return impl->socks5_port(); }

static Transport connect_internal(Settings settings, Logger *logger,
                                  Poller *poller) {

    if (settings.find("dumb_transport") != settings.end()) {
        return Transport(new Emitter(logger));
    }

    if (settings.find("family") == settings.end()) {
        settings["family"] = "PF_UNSPEC";
    }

    if (settings.find("address") == settings.end()) {
        throw std::runtime_error("invalid argument");
    }
    if (settings.find("port") == settings.end()) {
        throw std::runtime_error("invalid argument");
    }

    if (settings.find("socks5_proxy") != settings.end()) {
        auto proxy = settings["socks5_proxy"];
        auto pos = proxy.find(":");
        if (pos == std::string::npos) {
            throw std::runtime_error("invalid argument");
        }
        auto address = proxy.substr(0, pos);
        auto port = proxy.substr(pos + 1);
        settings["socks5_address"] = address;
        settings["socks5_port"] = port;
        return Transport(new Socks5(settings, logger, poller));
    }

    return Transport(new Connection(settings["family"].c_str(),
                                    settings["address"].c_str(),
                                    settings["port"].c_str(),
                                    logger, poller));
}

Maybe<Transport> connect(Settings settings, Logger *lp, Poller *poller) {
    double timeo = 30.0;
    if (settings.find("timeout") != settings.end()) {
        timeo = settings["timeout"].as<double>();
    }
    Transport transport = connect_internal(settings, lp, poller);
    if (timeo >= 0.0) {
        transport.set_timeout(timeo);
    }
    return Maybe<Transport>(transport);
}

void connect(std::string address, int port,
             std::function<void(Error, Transport)> callback,
             Settings settings, Logger *logger, Poller *poller) {
    net::connect(address, port, [callback](ConnectResult r) {
        // TODO: it would be nice to pass to this callback a compound error
        // that also contains info on all what went wrong when connecting
        if (r.overall_error) {
            callback(r.overall_error, nullptr);
            return;
        }
        callback(NoError(), new Connection(r.connected_bev));
    }, settings.get("timeout", 30.0), poller, logger);
}

} // namespace net
} // namespace mk
