// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_EMITTER_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_EMITTER_HPP

#include "src/libmeasurement_kit/common/delegate.hpp"
#include "src/libmeasurement_kit/dns/resolve_hostname.hpp"

#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class EmitterBase : public Transport {
  public:
    EmitterBase(SharedPtr<Reactor> reactor, SharedPtr<Logger> logger)
        : reactor(reactor), logger(logger) {}

    ~EmitterBase() override;

    /*
     * TransportEmitter
     */

    void emit_connect() override {
        logger->debug2("emitter: emit 'connect' event");
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        if (!do_connect) {
            logger->debug2("emitter: no handler set; ignoring");
            return;
        }
        do_connect();
    }

    void emit_data(Buffer data) override {
        logger->debug2("emitter: emit 'data' event "
                    "(num_bytes = %zu)", data.length());
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        if (do_record_received_data) {
            received_data_record.write(data.peek());
        }
        if (!do_data) {
            logger->debug2("emitter: no handler set; ignoring");
            return;
        }
        reactor->with_current_data_usage([&data](DataUsage &du) {
            du.down += data.length();
        });
        do_data(data);
    }

    void emit_flush() override {
        logger->debug2("emitter: emit 'flush' event");
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        if (!do_flush) {
            logger->debug2("emitter: no handler set; ignoring");
            return;
        }
        do_flush();
    }

    void emit_error(Error err) override {
        logger->debug2("emitter: emit 'error' event "
                    "(error = '%s')", err.what());
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        if (!do_error) {
            logger->debug2("emitter: no handler set; ignoring");
            return;
        }
        do_error(err);
    }

    void on_connect(std::function<void()> fn) override {
        logger->debug2("emitter: %sregister 'connect' handler",
                    (fn != nullptr) ? "" : "un");
        do_connect = fn;
    }

    void on_data(std::function<void(Buffer)> fn) override {
        logger->debug2("emitter: %sregister 'data' handler",
                    (fn != nullptr) ? "" : "un");
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        if (fn) {
            start_reading();
        } else {
            stop_reading();
        }
        do_data = fn;
    }

    void on_flush(std::function<void()> fn) override {
        logger->debug2("emitter: %sregister 'flush' handler",
                    (fn != nullptr) ? "" : "un");
        do_flush = fn;
    }

    void on_error(std::function<void(Error)> fn) override {
        logger->debug2("emitter: %sregister 'error' handler",
                    (fn != nullptr) ? "" : "un");
        do_error = fn;
    }

    void close(Callback<> cb) override;

    /*
     * TransportRecorder
     */

    void record_received_data() override {
        do_record_received_data = true;
    }

    void dont_record_received_data() override {
        do_record_received_data = false;
    }

    Buffer &received_data() override {
        return received_data_record;
    }

    void record_sent_data() override {
        do_record_sent_data = true;
    }

    void dont_record_sent_data() override {
        do_record_sent_data = false;
    }

    Buffer &sent_data() override {
        return sent_data_record;
    }

    /*
     * TransportWriter
     */

    void write(const void *p, size_t n) override {
        logger->debug2("emitter: send opaque data");
        if (p == nullptr) {
            throw std::runtime_error("null pointer");
        }
        write(Buffer(p, n));
    }

    void write(std::string s) override {
        logger->debug2("emitter: send string");
        write(Buffer(s));
    }

    void write(Buffer data) override {
        logger->debug2("emitter: send buffer");
        if (do_record_sent_data) {
            sent_data_record.write(data.peek());
        }
        reactor->with_current_data_usage([&data](DataUsage &du) {
            du.up += data.length();
        });
        output_buff << data;
        if (close_pending) {
            logger->debug2("emitter: already closed; ignoring");
            return;
        }
        start_writing();
    }

    /*
     * TransportSocks5
     */

    std::string socks5_address() override { return ""; }

    std::string socks5_port() override { return ""; }

    /*
     * TransportPollable
     */

    void set_timeout(double timeo) override {
        if (close_pending) {
            return;
        }
        adjust_timeout(timeo);
    }

    void clear_timeout() override { set_timeout(-1); }

    bufferevent *get_bufferevent() override {
        throw std::runtime_error("not_attached");
    }

    void set_bufferevent(bufferevent *) override {
        throw std::runtime_error("not_attached");
    }

    // Protected methods of TransportPollable: not implemented

    /*
     * TransportConnectable
     */

    double connect_time() override { return saved_connect_time; }
    void set_connect_time_(double x) override { saved_connect_time = x; }

    std::vector<Error> connect_errors() override {
        return saved_connect_errors;
    }
    void set_connect_errors_(std::vector<Error> x) override {
        saved_connect_errors = x;
    }

    dns::ResolveHostnameResult dns_result() override {
        return saved_dns_result;
    }
    void set_dns_result_(dns::ResolveHostnameResult x) override {
        saved_dns_result = x;
    }

    Endpoint sockname() override { return {}; }
    Endpoint peername() override { return {}; }

  protected:
    // TODO: it would probably better to have accessors
    SharedPtr<Reactor> reactor = Reactor::global();
    SharedPtr<Logger> logger = Logger::global();
    Buffer output_buff;

  private:
    Delegate<> do_connect;
    Delegate<Buffer> do_data;
    Delegate<> do_flush;
    Delegate<Error> do_error;
    bool do_record_received_data = false;
    Buffer received_data_record;
    bool do_record_sent_data = false;
    Buffer sent_data_record;
    Callback<> close_cb;
    bool close_pending = false;
    double saved_connect_time = 0.0;
    std::vector<Error> saved_connect_errors;
    dns::ResolveHostnameResult saved_dns_result;
};

class Emitter : public EmitterBase {
  public:
    Emitter(SharedPtr<Reactor> reactor, SharedPtr<Logger> logger)
        : EmitterBase(reactor, logger) {}

    ~Emitter() override;

  protected:
    void adjust_timeout(double timeo) override {
        logger->debug2("emitter: adjust_timeout %f", timeo);
    }

    void shutdown() override {}

    void start_reading() override {}
    void stop_reading() override {}
    void start_writing() override {}
};

} // namespace net
} // namespace mk
#endif
