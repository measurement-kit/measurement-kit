// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_CONNECTION_HPP
#define PRIVATE_LIBEVENT_CONNECTION_HPP

#include "private/net/emitter.hpp"
#include "private/net/utils.hpp"
#include "private/common/utils.hpp"
#include <cassert>
#include <cerrno>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/error_or.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/error.hpp>
#include <measurement_kit/net/transport.hpp>
#include <measurement_kit/net/utils.hpp>
#include <measurement_kit/portable/sys/socket.h>
#include <new>
#include <openssl/err.h>
#include <ostream>
#include <stdexcept>
#include <utility>

// TODO: we should probably rename this file `transport`.

extern "C" {

static inline void handle_libevent_read(bufferevent *, void *);
static inline void handle_libevent_write(bufferevent *, void *);
static inline void handle_libevent_event(bufferevent *, short, void *);

} // extern "C"

namespace mk {
namespace libevent {

using namespace mk::net;

// TODO: we should extend to support the connect event
static inline std::string map_bufferevent_event(short what) {
    std::stringstream ss;
    ss << ((what & BEV_EVENT_EOF) ? "Z" : "z")
       << ((what & BEV_EVENT_TIMEOUT) ? "T" : "t")
       << ((what & BEV_EVENT_ERROR) ? "F" : "f")
       << ((what & BEV_EVENT_READING) ? "R" : "r")
       << ((what & BEV_EVENT_WRITING) ? "W" : "w");
    return ss.str();
}

// TODO: we should write tests for this class
template <MK_MOCK(bufferevent_enable), MK_MOCK(bufferevent_disable)>
class Connection : public EmitterBase, public NonMovable, public NonCopyable {
  public:
    static Var<Transport> make(bufferevent *bev, Var<Reactor> reactor,
                               Var<Logger> logger) {
        Connection *conn = new Connection(bev, reactor, logger);
        conn->self = Var<Transport>(conn);
        return conn->self;
    }

    ~Connection() override {
        if (bev != nullptr) {
            bufferevent_free(bev);
        }
    }

  protected:
    void adjust_timeout(double timeout) override {
        timeval tv, *tvp = mk::timeval_init(&tv, timeout);
        bufferevent *underlying = bufferevent_get_underlying(this->bev);
        if (underlying) {
            // When we have a underlying bufferevent (i.e., a socket) set the
            // timeout to it rather than to the outer buffer because we have
            // seen running a long download that setting the timeout of the SSL
            // bufferevent leads to interrupted download due to timeout.
            if (bufferevent_set_timeouts(underlying, tvp, tvp) != 0) {
                throw std::runtime_error("cannot set timeout");
            }
            return;
        }
        if (bufferevent_set_timeouts(this->bev, tvp, tvp) != 0) {
            throw std::runtime_error("cannot set timeout");
        }
    }

    void start_writing() override {
        // Note: in theory writing _nonzero_ bytes into the output
        // bufferevent should be enough to start writing
        output_buff >> bufferevent_get_output(bev);
        if (bufferevent_enable(this->bev, EV_WRITE) != 0) {
            throw std::runtime_error("cannot enable write");
        }
    }

    void start_reading() override {
        if (bufferevent_enable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot enable read");
        }
    }

    void stop_reading() override {
        if (bufferevent_disable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot disable read");
        }
    }

    void stop_writing() override {
        if (bufferevent_disable(this->bev, EV_WRITE) != 0) {
            throw std::runtime_error("cannot disable write");
        }
    }

    void shutdown() override {
        if (shutdown_called) {
            return; // Just for extra safety
        }
        shutdown_called = true;
        bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr);
        reactor->call_soon([=]() { this->self = nullptr; });
    }

    template <decltype(getsockname) func> Endpoint sockname_peername_() {
        // Assumption: in the common case this operation won't fail. When it
        // fails, we'll just return an empty endpoint.
        assert(bev != nullptr);
        auto fd = bufferevent_getfd(bev);
        if (fd == -1) {
            logger->warn("connection: bufferevent attached to invalid socket");
            return {};
        }
        sockaddr_storage ss{};
        socklen_t sslen = sizeof(ss);
        if (func(fd, (sockaddr *)&ss, &sslen) != 0) {
            logger->warn("connection: cannot get socket name / peer name");
            return {};
        }
        ErrorOr<Endpoint> epnt = endpoint_from_sockaddr_storage(&ss);
        if (!epnt) {
            logger->warn("connection: cannot get endpoint from "
                         "sockaddr_storage structure");
            return {};
        }
        return *epnt;
    }

    Endpoint sockname() override { return sockname_peername_<::getsockname>(); }

    Endpoint peername() override { return sockname_peername_<::getpeername>(); }

  public:
    // They MUST be public because they're called by C code

    void handle_event_(short what) {
        // Implementation note: The event for which we're called are:
        //
        // 1. EOF while reading with `(what & BEV_EVENT_EOF) != 0`
        // 2. libevent timeout with `(what & BEV_EVENT_TIMEOUT) != 0`
        // 3. SSL or socket error with `(what & BEV_EVENT_ERROR) != 0`
        // 4. successful read with `what == BEV_EVENT_READING`
        // 5. successful flush with `what == BEV_EVENT_WRITING`
        // 6. successful connect with `what == BEV_EVENT_CONNECTED`
        //
        // In cases 1-3, only one of the reading, writing, and connected flags
        // should bet set. But sometimes the filters forget about that.

        Error err = NoError();

        logger->debug("connection: got bufferevent event: %s",
                      map_bufferevent_event(what).c_str());

        if ((what & BEV_EVENT_EOF) != 0) {
            // Implementation note: the following is a fix because we have seen
            // filter SSL bufferevents deliver EOF _before_ data. It was with
            // the v2.0.x branch of libevent and _may_ now be fixed. IIRC,
            // to test we need to run against a server that performs a "dirty"
            // SSL shutdown (i.e. just closes the socket, no SSL_shutdown).
            auto input = bufferevent_get_input(bev);
            if (evbuffer_get_length(input) > 0) {
                logger->debug(
                      "Suppress EOF with data lingering in input buffer");
                suppressed_eof = true;
                return;
            }
            err = EofError();
        }

        if ((what & BEV_EVENT_TIMEOUT) != 0) {
            err = TimeoutError();
        }

        if ((what & BEV_EVENT_ERROR) != 0) {
            err = net::map_errno(errno);
            if (err == NoError()) {
                unsigned long ossl_err;
                char buff[128];
                while ((ossl_err = bufferevent_get_openssl_error(bev)) != 0) {
                    err = (!!err) ? err : SslError();
                    ERR_error_string_n(ossl_err, buff, sizeof(buff));
                    err.add_child_error(SslError(buff));
                }
                if (err != SslError()) {
                    /*
                     * This is the case of the SSL dirty shutdown:
                     *
                     * The connection was not closed cleanly from the other end.
                     * In theory this could also be the effect of an attack.
                     */
                    logger->warn("libevent has detected an SSL dirty shutdown");
                    err = SslDirtyShutdownError();
                }
            }
            logger->warn("Got error: %s", err.as_ooni_error().c_str());
        }

        // Implementation note: for now, we don't handle the connect event
        // in this callback. When we will do that, we will have the problem
        // that in some cases libevent does not set operation flags, so we
        // may end up emitting connect when it doesn't make sense.
        if ((what & BEV_EVENT_CONNECTED) != 0) {
            logger->warn("received unexpected connected event");
            return;
        }

        // If we have got an error and libevent doesn't know what it was doing,
        // wake up both the read and the flush handlers. Of course, if the
        // programmer didn't register any handler, nothing will really happen.
        if (!!err && (what & (BEV_EVENT_READING | BEV_EVENT_WRITING)) == 0) {
            what |= BEV_EVENT_READING | BEV_EVENT_WRITING;
        }

        if ((what & BEV_EVENT_READING) != 0) {
            Buffer buff(bufferevent_get_input(bev));
            if (suppressed_eof) {
                suppressed_eof = false;
                logger->debug("Deliver previously suppressed EOF");
                err = (!!err) ? err : EofError();
            }
            // XXX previously here we were catching errors
            emit_data(err, buff);
        }
        
        if ((what & BEV_EVENT_WRITING) != 0) {
            // XXX previously here we were catching errors
            emit_flush(err);
        }
    }

  private:
    Connection(bufferevent *bev, Var<Reactor> reactor, Var<Logger> logger)
        : EmitterBase{reactor, logger} {

        this->bev = bev;

        // The following makes this non copyable and non movable.
        bufferevent_setcb(this->bev, handle_libevent_read,
                          handle_libevent_write, handle_libevent_event, this);
    }

    bufferevent *bev = nullptr;
    Var<Transport> self;
    Callback<> close_cb;
    bool suppressed_eof = false;
    bool shutdown_called = false;
};

} // namespace libevent
} // namespace mk
#endif

extern "C" {

static inline void handle_libevent_read(bufferevent *, void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_event_(
        BEV_EVENT_READING);
}

static inline void handle_libevent_write(bufferevent *, void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_event_(
        BEV_EVENT_WRITING);
}

static inline void handle_libevent_event(bufferevent *, short what,
                                         void *opaque) {
    static_cast<mk::libevent::Connection *>(opaque)->handle_event_(what);
}

} // extern "C"
