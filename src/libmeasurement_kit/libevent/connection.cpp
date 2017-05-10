// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/connection.hpp"
#include "../net/utils.hpp"

#include <measurement_kit/net.hpp>

#include <event2/buffer.h>
#include <event2/dns.h>

#include <cerrno>
#include <new>

extern "C" {

static void handle_libevent_read(bufferevent *, void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_read_();
}

static void handle_libevent_write(bufferevent *, void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_write_();
}

static void handle_libevent_event(bufferevent *, short what, void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_event_(what);
}

} // extern "C"
namespace mk {
namespace libevent {

using namespace mk::net;

void Connection::handle_read_() {
    Buffer buff(bufferevent_get_input(bev));
    try {
        emit_data(buff);
    } catch (Error &error) {
        emit_error(error);
        return;
    }
    if (suppressed_eof) {
        suppressed_eof = false;
        logger->debug("Deliver previously suppressed EOF");
        emit_error(EofError());
        return;
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
    logger->debug("connection: got bufferevent event: %d", what);

    if (what & BEV_EVENT_EOF) {
        auto input = bufferevent_get_input(bev);
        if (evbuffer_get_length(input) > 0) {
            logger->debug("Suppress EOF with data lingering in input buffer");
            suppressed_eof = true;
            return;
        }
        emit_error(EofError());
        return;
    }

    if (what & BEV_EVENT_TIMEOUT) {
        emit_error(TimeoutError());
        return;
    }

    Error sys_error = net::map_errno(errno);
    logger->warn("Got error: %s", sys_error.as_ooni_error().c_str());
    emit_error(sys_error);
}

Connection::Connection(bufferevent *buffev, Var<Reactor> reactor, Var<Logger> logger)
        : EmitterBase(reactor, logger) {
    this->bev = buffev;

    // The following makes this non copyable and non movable.
    bufferevent_setcb(this->bev, handle_libevent_read, handle_libevent_write,
                      handle_libevent_event, this);
}

void Connection::shutdown() {
    if (shutdown_called) {
        return; // Just for extra safety
    }
    shutdown_called = true;
    bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
    reactor->call_soon([=]() {
        this->self = nullptr;
    });
}

} // namespace libevent
} // namespace mk
