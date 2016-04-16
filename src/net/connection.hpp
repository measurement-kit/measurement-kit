// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECTION_HPP
#define SRC_NET_CONNECTION_HPP

#include <event2/bufferevent.h>
#include <event2/event.h>
#include <list>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/net.hpp>
#include <stdexcept>
#include <string.h>
#include "src/common/utils.hpp"
#include "src/net/bufferevent.hpp"
#include "src/net/emitter.hpp"

namespace mk {
namespace net {

class Connection : public Emitter, public NonMovable, public NonCopyable {
  public:
    Connection(bufferevent *bev, Logger * = Logger::global());

    ~Connection() override {}

    evutil_socket_t get_fileno() { return (bufferevent_getfd(this->bev)); }

    void set_timeout(double timeout) override {
        struct timeval tv, *tvp;
        tvp = mk::timeval_init(&tv, timeout);
        if (bufferevent_set_timeouts(this->bev, tvp, tvp) != 0) {
            throw std::runtime_error("cannot set timeout");
        }
    }

    void clear_timeout() override { this->set_timeout(-1); }

    void start_tls(unsigned int) {
        throw std::runtime_error("not implemented");
    }

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

    void close() override;

    void handle_event_(short);
    void handle_read_();
    void handle_write_();

  private:
    Bufferevent bev;
};

} // namespace net
} // namespace mk
#endif
