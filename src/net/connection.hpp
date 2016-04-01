// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_CONNECTION_HPP
#define SRC_NET_CONNECTION_HPP

#include <event2/bufferevent.h>
#include <event2/event.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/net.hpp>
#include "src/common/delayed_call.hpp"
#include "src/common/utils.hpp"
#include "src/net/bufferevent.hpp"
#include "src/net/emitter.hpp"
#include <list>
#include <stdexcept>
#include <string.h>

namespace mk {
namespace net {

class Connection : public Emitter, public NonMovable, public NonCopyable {
  public:
    Connection(bufferevent *bev);

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

    void send(Buffer data) override { data >> bufferevent_get_output(bev); }

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

  private:
    Bufferevent bev;
    Poller *poller = Poller::global();

    // Libevent callbacks
    static void handle_read(bufferevent *, void *);
    static void handle_write(bufferevent *, void *);
    static void handle_event(bufferevent *, short, void *);

    // Stuff for connecting (later this will be removed):

  public:
    Connection(evutil_socket_t fd, Logger *lp = Logger::global(),
               Poller *poller = mk::get_global_poller())
        : Connection("PF_UNSPEC", "0.0.0.0", "0", poller, lp, fd) {}

    Connection(const char *af, const char *a, const char *p,
               Logger *lp = Logger::global(),
               Poller *poller = mk::get_global_poller())
        : Connection(af, a, p, poller, lp, -1) {}

    Connection(const char *, const char *, const char *, Poller *, Logger *,
               evutil_socket_t);

  private:
    unsigned int connecting = 0;
    std::string address;
    std::string port;
    std::list<std::pair<std::string, std::string>> addrlist;
    std::string family;
    unsigned int must_resolve_ipv4 = 0;
    unsigned int must_resolve_ipv6 = 0;
    DelayedCall start_connect;

    void connect_next();
    void handle_resolve(Error, std::vector<dns::Answer>);
    void resolve();
    bool resolve_internal(char);
};

} // namespace net
} // namespace mk
#endif
