// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_LIBEVENT_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_LIBEVENT_HPP

#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/net/utils.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
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
#include "src/libmeasurement_kit/net/buffer.hpp"
#include "src/libmeasurement_kit/net/error.hpp"
#include "src/libmeasurement_kit/net/transport.hpp"
#include "src/libmeasurement_kit/net/utils.hpp"
#include <measurement_kit/common/aaa_base.h>
#include <new>
#include <openssl/err.h>
#include <ostream>
#include <stdexcept>
#include <utility>

extern "C" {

static inline void handle_libevent_read(bufferevent *, void *);
static inline void handle_libevent_write(bufferevent *, void *);
static inline void handle_libevent_event(bufferevent *, short, void *);

} // extern "C"

namespace mk {
namespace net {

static inline std::string map_bufferevent_event(short what) {
    std::stringstream ss;
    ss << ((what & BEV_EVENT_EOF) ? "Z" : "z")
       << ((what & BEV_EVENT_TIMEOUT) ? "T" : "t")
       << ((what & BEV_EVENT_ERROR) ? "F" : "f")
       << ((what & BEV_EVENT_READING) ? "R" : "r")
       << ((what & BEV_EVENT_WRITING) ? "W" : "w");
    return ss.str();
}

class LibeventEmitter : public EmitterBase, public NonMovable, public NonCopyable {
  public:
    static SharedPtr<Transport> make(bufferevent *bev, SharedPtr<Reactor> reactor,
                               SharedPtr<Logger> logger) {
        LibeventEmitter *conn = new LibeventEmitter(bev, reactor, logger);
        conn->self = SharedPtr<Transport>(conn);
        return conn->self;
    }

    ~LibeventEmitter() override {
        if (bev != nullptr) {
            bufferevent_free(bev);
        }
    }

    bufferevent *get_bufferevent() override {
        return bev;
    }

    void set_bufferevent(bufferevent *new_bev) override {
        bev = new_bev;
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
        output_buff >> bufferevent_get_output(bev);
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

        logger->debug("connection: got bufferevent event: %s",
                      map_bufferevent_event(what).c_str());

        if ((what & BEV_EVENT_EOF) != 0) {
            auto input = bufferevent_get_input(bev);
            if (evbuffer_get_length(input) > 0) {
                logger->debug(
                      "Suppress EOF with data lingering in input buffer");
                suppressed_eof = true;
                return;
            }
            emit_error(EofError());
            return;
        }

        if ((what & BEV_EVENT_TIMEOUT) != 0) {
            emit_error(TimeoutError());
            return;
        }

        Error sys_error = net::map_errno(errno);

        if (sys_error == NoError()) {
            unsigned long openssl_error;
            char buff[128];
            while ((openssl_error = bufferevent_get_openssl_error(bev)) != 0) {
                if (sys_error == NoError()) {
                    sys_error = SslError();
                }
                ERR_error_string_n(openssl_error, buff, sizeof(buff));
                sys_error.add_child_error(SslError(buff));
            }
            if (sys_error != SslError()) {
                /*
                 * This is the case of the SSL dirty shutdown. The connection
                 * was not closed cleanly from the other end and in theory this
                 * could also be the effect of an attack.
                 */
                logger->warn("libevent has detected an SSL dirty shutdown");
                sys_error = SslDirtyShutdownError();
            }
        }

        logger->warn("Got error: %s", sys_error.what());
        emit_error(sys_error);
    }

    void handle_read_() {
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

    void handle_write_() {
        try {
            emit_flush();
        } catch (Error &error) {
            emit_error(error);
        }
    }

  private:
    LibeventEmitter(bufferevent *bev, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger)
        : EmitterBase{reactor, logger} {

        this->bev = bev;

        // The following makes this non copyable and non movable.
        bufferevent_setcb(this->bev, handle_libevent_read,
                          handle_libevent_write, handle_libevent_event, this);
    }

    bufferevent *bev = nullptr;
    SharedPtr<Transport> self;
    Callback<> close_cb;
    bool suppressed_eof = false;
    bool shutdown_called = false;
};

} // namespace net
} // namespace mk
#endif

extern "C" {

static inline void handle_libevent_read(bufferevent *, void *opaque) {
    static_cast<mk::net::LibeventEmitter *>(opaque)->handle_read_();
}

static inline void handle_libevent_write(bufferevent *, void *opaque) {
    static_cast<mk::net::LibeventEmitter *>(opaque)->handle_write_();
}

static inline void handle_libevent_event(bufferevent *, short what,
                                         void *opaque) {
    static_cast<mk::net::LibeventEmitter *>(opaque)->handle_event_(what);
}

} // extern "C"
