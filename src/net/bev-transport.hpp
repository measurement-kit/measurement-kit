// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_BEV_TRANSPORT_HPP
#define SRC_NET_BEV_TRANSPORT_HPP

#include <functional>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/evbuffer.hpp>
#include <measurement_kit/common/func.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/net/buffer.hpp>
#include <stddef.h>
#include <string>
#include <sys/time.h>
#include "src/common/utils.hpp"
#include "src/net/bev-wrappers.hpp"

// Forward declarations
struct bufferevent;

namespace mk {
namespace net {

class TransportBev {
  public:
    static Var<TransportBev> create(Var<bufferevent> buffev,
                                    Poller * = Poller::global(),
                                    Logger * = Logger::global());

    void emit_connect() { if (connect_cb) connect_cb(); }

    void emit_data(Buffer data) { if (data_cb) data_cb(data); }

    void emit_flush() { if (flush_cb) flush_cb(); }

    void emit_error(Error error) { if (error_cb) error_cb(error); }

    void on_connect(std::function<void()> cb) { connect_cb = cb; }

    void on_data(std::function<void(Buffer)> cb) { data_cb = cb; }

    void on_flush(std::function<void()> cb) { flush_cb = cb; }

    void on_error(std::function<void(Error)> cb) { error_cb = cb; }

    void set_timeout(double timeo) {
        timeval tv, *tvp = timeval_init(&tv, timeo);
        mk::net::bufferevent_set_timeouts(bev, tvp, tvp);
    }

    void clear_timeout() { set_timeout(-1.0); }

    void send(const void *base, size_t len) {
        mk::net::bufferevent_write(bev, base, len);
    }

    void send(std::string str) { send(str.data(), str.size()); }

    void send(Buffer data) {
        mk::net::bufferevent_write_buffer(bev, *data.as_evbuffer());
    }

    // Public such that we can access them from C code
    Var<bufferevent> bev;
    Func<void()> connect_cb;
    Func<void(net::Buffer)> data_cb;
    Func<void(Error)> error_cb;
    Func<void()> flush_cb;
    Logger *logger = Logger::global();
    Poller *poller = Poller::global();
    Var<TransportBev> self;
};

/// Connect transport according to net/transport.hpp API
/// \param settings Setting used to connect. This function requires the
///        `address` and `port` field to be set.
/// \param poller Optional poller, otherwise the global one is used.
/// \param logger Optional logger, otherwise the global one is used.
/// \return A transport whose connection is in progress.
/// \bug DNS resolution is not asynchronous.
Var<TransportBev> connect(Settings, Poller * = Poller::global(),
                          Logger * = Logger::global());

} // namespace net
} // namespace mk
#endif
