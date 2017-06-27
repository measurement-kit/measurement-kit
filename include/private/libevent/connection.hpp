// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_CONNECTION_HPP
#define PRIVATE_LIBEVENT_CONNECTION_HPP

#include "../net/utils.hpp"
#include "../net/emitter.hpp"

#include <measurement_kit/net.hpp>

#include <event2/bufferevent.h>
#include <event2/event.h>

namespace mk {
namespace libevent {

using namespace mk::net;

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

    void shutdown() override;

    // They MUST be public because they're called by C code
  public:
    void handle_event_(short);
    void handle_read_();
    void handle_write_();

  private:
    Connection(bufferevent *bev, Var<Reactor>, Var<Logger>);

    bufferevent *bev = nullptr;
    Var<Transport> self;
    Callback<> close_cb;
    bool suppressed_eof = false;
    bool shutdown_called = false;
};

} // namespace libevent
} // namespace mk
#endif
