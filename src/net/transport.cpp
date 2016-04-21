// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <cassert>
#include <measurement_kit/net.hpp>
#include "src/net/connect.hpp"
#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include "src/net/socks5.hpp"
#include "src/net/ssl-context.hpp"

namespace mk {
namespace net {

// TODO: this should be moved into src/net/connect.cpp
void connect(std::string address, int port,
             Callback<Var<Transport>> callback,
             Settings settings, Logger *logger, Poller *poller) {
    if (settings.find("dumb_transport") != settings.end()) {
        callback(NoError(), Var<Transport>(new Emitter(logger)));
        return;
    }
    if (settings.find("socks5_proxy") != settings.end()) {
        socks5_connect(address, port, settings, callback, poller, logger);
        return;
    }
    double timeout = settings.get("timeout", 30.0);
    connect_logic(address, port, [=](Error err, Var<ConnectResult> r) {
        if (err) {
            err.context = r;
            callback(err, nullptr);
            return;
        }
        if (settings.find("ssl") != settings.end()) {
            SslContext *ssl_context;
            if (settings.find("ca_bundle_path") != settings.end()) {
                std::string ca_bundle_path = settings.at("ca_bundle_path");
                logger->debug("ssl: using custom ca_bundle_path: %s", ca_bundle_path.c_str());
                ssl_context = new SslContext(ca_bundle_path);
            } else {
                ssl_context = SslContext::default_context();
            }
            connect_ssl(r->connected_bev, ssl_context->get_client_ssl(address), address,
                        [=](Error err, bufferevent *bev) {
                            if (err) {
                                err.context = r;
                                callback(err, nullptr);
                                return;
                            }
                            Var<Transport> txp = Connection::make(bev,
                                    poller, logger);
                            txp->set_timeout(timeout);
                            assert(err == NoError());
                            err.context = r;
                            callback(err, txp);
                        },
                        poller, logger);
            return;
        }
        Var<Transport> txp = Connection::make(r->connected_bev, poller, logger);
        txp->set_timeout(timeout);
        assert(err == NoError());
        err.context = r;
        callback(err, txp);
    }, timeout, poller, logger);
}

} // namespace net
} // namespace mk
