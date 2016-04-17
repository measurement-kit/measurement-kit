// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECTION_HPP
#define SRC_NET_CONNECTION_HPP

#include "src/common/utils.hpp"
#include "src/net/bufferevent.hpp"
#include "src/net/emitter.hpp"
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <list>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/net.hpp>
#include <stdexcept>
#include <string.h>

namespace mk {
namespace net {

class Connection : public Emitter, public NonMovable, public NonCopyable {
  public:
    static Var<Transport> make(bufferevent *bev,
                               Poller *poller = Poller::global(),
                               Logger *logger = Logger::global()) {
        Connection *conn = new Connection(bev, poller, logger);
        conn->self = Var<Transport>(conn);
        return conn->self;
    }

    ~Connection() override {
        // Nothing for now
    }

    void set_timeout(double timeout) override {
        timeval tv, *tvp = mk::timeval_init(&tv, timeout);
        if (bufferevent_set_timeouts(this->bev, tvp, tvp) != 0) {
            throw std::runtime_error("cannot set timeout");
        }
    }

    void clear_timeout() override { set_timeout(-1); }

    void do_send(Buffer data) override { data >> bufferevent_get_output(bev); }

    void enable_read() override {
        if (bufferevent_enable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot enable read");
        }
    }

    void disable_read() override {
        if (bufferevent_disable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot disable read");
        }
    }

    void close(std::function<void()>) override;

    void handle_event_(short);
    void handle_read_();
    void handle_write_();

#define safe_upcall(ptr_, method_and_args_)                                    \
    do {                                                                       \
        ptr_->method_and_args_;                                                \
    } while (0)
    static void emit_libevent_event_(Connection *me, short what) {
        safe_upcall(me, handle_event_(what));
    }
    static void emit_libevent_read_(Connection *me) {
        safe_upcall(me, handle_read_());
    }
    static void emit_libevent_write_(Connection *me) {
        safe_upcall(me, handle_write_());
    }
#undef safe_upcall

  private:
    Connection(bufferevent *bev, Poller * = Poller::global(),
               Logger * = Logger::global());

    Bufferevent bev;
    Var<Transport> self;
    Poller *poller = Poller::global();
    bool isclosed = false;
};

} // namespace net
} // namespace mk
#endif
