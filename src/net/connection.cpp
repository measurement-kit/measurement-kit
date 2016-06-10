// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/inet.h>
#include <errno.h>
#include <event2/dns.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <new>
#include <stdlib.h>
#include "src/net/connection.hpp"

extern "C" {

static void handle_libevent_read(bufferevent *, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_read_();
}

static void handle_libevent_write(bufferevent *, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_write_();
}

static void handle_libevent_event(bufferevent *, short what, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_event_(what);
}

} // extern "C"
namespace mk {
namespace net {

void Connection::handle_read_() {
    Buffer buff(bufferevent_get_input(bev));
    try {
        emit_data(buff);
    } catch (Error &error) {
        emit_error(error);
    }
}

void Connection::handle_write_() {
    try {
        emit_flush();
    } catch (Error &error) {
        emit_error(error);
    }
}

void Connection::handle_event_(short what) {

    if (what & BEV_EVENT_EOF) {
        emit_error(EofError());
        return;
    }

    if (what & BEV_EVENT_TIMEOUT) {
        emit_error(TimeoutError());
        return;
    }

    if (errno == EPIPE) {
        emit_error(BrokenPipeError());
        return;
    }

    // TODO: Here we need to map more network errors

    emit_error(SocketError());
}

Connection::Connection(bufferevent *buffev, Var<Reactor> reactor, Var<Logger> logger)
        : Emitter(logger), reactor(reactor) {
    this->bev = buffev;

    // The following makes this non copyable and non movable.
    bufferevent_setcb(this->bev, handle_libevent_read, handle_libevent_write,
                      handle_libevent_event, this);
}

void Connection::close(std::function<void()> cb) {
    if (isclosed) {
        throw std::runtime_error("already closed");
    }
    isclosed = true;

    on_connect(nullptr);
    on_data(nullptr);
    on_flush(nullptr);
    on_error(nullptr);
    bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
    disable_read();

    close_cb = cb;
    reactor->call_soon([=]() {
        this->self = nullptr;
    });
}

} // namespace net
} // namespace mk
