// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <functional>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/dns/response.hpp>
#include <measurement_kit/net/error.hpp>
#include "src/dns/getaddrinfo.hpp"
#include "src/net/bev-connect.hpp"
#include "src/net/bev-transport.hpp"
#include "src/net/bev-wrappers.hpp"

// Forward declarations
struct bufferevent;
struct evbuffer;

extern "C" {

using namespace mk::net;
using namespace mk;

static void do_gc(Var<TransportBev> tbev) {
    tbev->logger->debug("garbage collecting unused tbev=%p...", tbev.get());
    net::bufferevent_disable(tbev->bev, EV_READ | EV_WRITE);
    ::bufferevent_setcb(tbev->bev.get(), nullptr, nullptr, nullptr, nullptr);
    tbev->poller->call_soon([tbev]() {
        tbev->logger->debug("garbage collecting tbev=%p... done", tbev.get());
        tbev->self = nullptr; // Remove self reference
    });
}

static void event_cb(bufferevent *, short what, void *opaque) {
    Var<TransportBev> tbev = static_cast<TransportBev *>(opaque)->self;

    // Make sure we pass all input upstream
    evbuffer *input = net::bufferevent_get_input(tbev->bev);
    if (evbuffer_get_length(input) > 0) {
        tbev->emit_data(input);
    }

    // Pass the error that occurred upstream
    if (what & BEV_EVENT_TIMEOUT) {
        tbev->emit_error(net::TimeoutError());
    } else if (what & BEV_EVENT_EOF) {
        tbev->emit_error(net::EOFError());
    } else {
        tbev->emit_error(net::SocketError());
    }

    // Garbage collect if upstream is not interested anymore
    if (tbev.unique()) {
        do_gc(tbev);
    }
}

static void read_cb(bufferevent *, void *opaque) {
    Var<TransportBev> tbev = static_cast<TransportBev *>(opaque)->self;
    tbev->emit_data(net::bufferevent_get_input(tbev->bev));
    if (tbev.unique()) {
        do_gc(tbev);
    }
}

static void write_cb(bufferevent *, void *opaque) {
    Var<TransportBev> tbev = static_cast<TransportBev *>(opaque)->self;
    tbev->emit_flush();
    if (tbev.unique()) {
        do_gc(tbev);
    }
}

} // extern "C"
namespace mk {
namespace net {

static void attach(Var<TransportBev> tbev, Var<bufferevent> bev) {
    tbev->bev = bev;
    ::bufferevent_setcb(bev.get(), read_cb, write_cb, event_cb, tbev.get());
    tbev->self = tbev; // Add self reference
    mk::net::bufferevent_enable(bev, EV_READ);
}

Var<TransportBev> TransportBev::create(Var<bufferevent> bev, Poller *poller,
                                       Logger *logger) {
    Var<TransportBev> tbev(new TransportBev);
    tbev->poller = poller;
    tbev->logger = logger;
    attach(tbev, bev);
    return tbev;
}

Var<TransportBev> connect(Settings settings, Poller *poller, Logger *logger) {
    static const double timeout = 30.0;
    Var<TransportBev> tbev(new TransportBev);
    tbev->poller = poller;
    tbev->logger = logger;
    // Connect may complete immediately on BSD systems. We don't want this
    // to happen because caller has not had change to set callbacks. So delay.
    poller->call_soon([logger, poller, settings, tbev]() {
        // XXX: resolving domain names is blocking
        // XXX: setting a default timeout
        Error error = connect_dns_sync(
            settings.at("address"),
            settings.at("port"), [tbev](Error error, Var<bufferevent> bev) {
                if (error) {
                    tbev->emit_error(error);
                    return;
                }
                attach(tbev, bev);
                tbev->emit_connect();
            }, timeout, poller, logger);
        if (error) {
            tbev->emit_error(error);
            return;
        }
    });
    return tbev;
}

} // namespace net
} // namespace mk
