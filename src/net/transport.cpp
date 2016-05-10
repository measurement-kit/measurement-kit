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
             Callback<Error, Var<Transport>> callback,
             Settings settings, Var<Logger> logger, Var<Reactor> reactor) {
    if (settings.find("net/dumb_transport") != settings.end()) {
        callback(NoError(), Var<Transport>(new Emitter(logger)));
        return;
    }
    if (settings.find("net/socks5_proxy") != settings.end()) {
        socks5_connect(address, port, settings, callback, reactor, logger);
        return;
    }
    double timeout = settings.get("net/timeout", 30.0);
    connect_logic(address, port, [=](Error err, Var<ConnectResult> r) {
        if (err) {
            err.context = r;
            callback(err, nullptr);
            return;
        }
        if (settings.find("net/ssl") != settings.end()) {
            Var<SslContext> ssl_context;
            if (settings.find("net/ca_bundle_path") != settings.end()) {
                logger->debug("ssl: using custom ca_bundle_path");
                ssl_context = Var<SslContext>(new SslContext(settings.at("net/ca_bundle_path")));
            } else {
                logger->debug("ssl: using default context");
                ssl_context = SslContext::global();
            }
            connect_ssl(r->connected_bev, ssl_context->get_client_ssl(address), address,
                        [r, callback, timeout, ssl_context, reactor, logger](Error err, bufferevent *bev) {
                            if (err) {
                                err.context = r;
                                callback(err, nullptr);
                                return;
                            }
                            Var<Transport> txp = Connection::make(bev,
                                    reactor, logger);
                            txp->set_timeout(timeout);
                            assert(err == NoError());
                            err.context = r;
                            callback(err, txp);
                        },
                        reactor, logger);
            return;
        }
        Var<Transport> txp = Connection::make(r->connected_bev, reactor, logger);
        txp->set_timeout(timeout);
        assert(err == NoError());
        err.context = r;
        callback(err, txp);
    }, settings, reactor, logger);
}

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb) {
    txp->on_flush([=]() {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        cb(NoError());
    });
    txp->on_error([=](Error err) {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        cb(err);
    });
    txp->write(buf);
}

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb) {
    if (buff->length() >= n) {
        // Shortcut that simplifies coding a great deal
        cb(NoError());
        return;
    }
    txp->on_data([=](Buffer d) {
        *buff << d;
        if (buff->length() < n) {
            return;
        }
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        cb(NoError());
    });
    txp->on_error([=](Error error) {
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        cb(error);
    });
}

} // namespace net
} // namespace mk
