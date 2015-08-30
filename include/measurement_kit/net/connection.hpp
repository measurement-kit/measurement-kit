// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_CONNECTION_HPP
#define MEASUREMENT_KIT_NET_CONNECTION_HPP

#include <measurement_kit/common/bufferevent.hpp>
#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/delayed_call.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/string_vector.hpp>
#include <measurement_kit/common/utils.hpp>

#include <measurement_kit/net/buffer.hpp>
#include <measurement_kit/net/transport.hpp>

#include <measurement_kit/dns/dns.hpp>

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <stdexcept>
#include <string.h>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

class Connection : public Transport {
  private:
    Bufferevent bev;
    dns::Request dns_request;
    unsigned int connecting = 0;
    char *address = NULL;
    char *port = NULL;
    StringVector *addrlist = NULL;
    char *family = NULL;
    StringVector *pflist = NULL;
    unsigned int must_resolve_ipv4 = 0;
    unsigned int must_resolve_ipv6 = 0;
    SharedPointer<DelayedCall> start_connect;
    Logger *logger = Logger::global();

    // Libevent callbacks
    static void handle_read(bufferevent *, void *);
    static void handle_write(bufferevent *, void *);
    static void handle_event(bufferevent *, short, void *);

    // Functions used when connecting
    void connect_next();
    void handle_resolve(int, char, std::vector<std::string>);
    void resolve();
    bool resolve_internal(char);

    std::function<void()> on_connect_fn = []() {
        /* nothing */
    };

    std::function<void()> on_ssl_fn = []() {
        /* nothing */
    };

    std::function<void(SharedPointer<Buffer>)> on_data_fn =
        [](SharedPointer<Buffer>) {
        /* nothing */
    };

    std::function<void()> on_flush_fn = []() {
        /* nothing */
    };

    std::function<void(Error)> on_error_fn = [](Error) {
        /* nothing */
    };

  public:
    Connection(evutil_socket_t fd, Logger *lp = Logger::global(),
               Poller *poller = measurement_kit::get_global_poller())
        : Connection("PF_UNSPEC", "0.0.0.0", "0", poller, lp, fd) {}

    Connection(const char *af, const char *a, const char *p,
               Logger *lp = Logger::global(),
               Poller *poller = measurement_kit::get_global_poller())
        : Connection(af, a, p, poller, lp, -1) {}

    Connection(const char *, const char *, const char *, Poller *,
               Logger *, evutil_socket_t);

    ~Connection() override;

    void on_connect(std::function<void()> fn) override { on_connect_fn = fn; };

    void on_ssl(std::function<void()> fn) override { on_ssl_fn = fn; };

    void on_data(std::function<void(SharedPointer<Buffer>)> fn) override {
        on_data_fn = fn;
        enable_read();
    };

    void on_flush(std::function<void()> fn) override { on_flush_fn = fn; };

    void on_error(std::function<void(Error)> fn) override { on_error_fn = fn; };

    evutil_socket_t get_fileno() { return (bufferevent_getfd(this->bev)); }

    void set_timeout(double timeout) override {
        struct timeval tv, *tvp;
        tvp = measurement_kit::timeval_init(&tv, timeout);
        if (bufferevent_set_timeouts(this->bev, tvp, tvp) != 0) {
            throw std::runtime_error("cannot set timeout");
        }
    }

    void clear_timeout() override { this->set_timeout(-1); }

    void start_tls(unsigned int) {
        throw std::runtime_error("not implemented");
    }

    void send(const void *base, size_t count) override {
        if (base == NULL || count == 0) {
            throw std::runtime_error("invalid argument");
        }
        if (bufferevent_write(bev, base, count) != 0) {
            throw std::runtime_error("cannot write");
        }
    }

    void send(std::string data) override { send(data.c_str(), data.length()); }

    void send(SharedPointer<Buffer> data) override { send(*data); }

    void send(Buffer &data) override { data >> bufferevent_get_output(bev); }

    void enable_read() {
        if (bufferevent_enable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot enable read");
        }
    }

    void disable_read() {
        if (bufferevent_disable(this->bev, EV_READ) != 0) {
            throw std::runtime_error("cannot disable read");
        }
    }

    void close() override;

    void emit_connect() override { on_connect_fn(); }

    void emit_data(SharedPointer<Buffer> data) override { on_data_fn(data); }

    void emit_flush() override { on_flush_fn(); }

    void emit_error(Error err) override { on_error_fn(err); }

    std::string socks5_address() override { return ""; /* not proxy */ }

    std::string socks5_port() override { return ""; /* not proxy */ }
};

}}
#endif
