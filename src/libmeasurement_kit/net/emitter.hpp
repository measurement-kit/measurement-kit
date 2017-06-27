// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NET_EMITTER_HPP
#define SRC_LIBMEASUREMENT_KIT_NET_EMITTER_HPP

#include <measurement_kit/net.hpp>

namespace mk {
namespace net {

class EmitterBase : public Transport {
  public:
    EmitterBase(Var<Reactor> reactor, Var<Logger> logger)
        : reactor(reactor), logger(logger) {}

    ~EmitterBase() override;

    /*
     * TransportEmitter
     */

    void emit_connect() override {
        logger->log(MK_LOG_DEBUG2, "emitter: emit 'connect' event");
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
            return;
        }
        if (!do_connect) {
            logger->log(MK_LOG_DEBUG2, "emitter: no handler set; ignoring");
            return;
        }
        do_connect();
    }

    void emit_data(Buffer data) override {
        logger->log(MK_LOG_DEBUG2, "emitter: emit 'data' event "
                    "(num_bytes = %lu)", data.length());
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
            return;
        }
        if (do_record_received_data) {
            received_data_record.write(data.peek());
        }
        if (!do_data) {
            logger->log(MK_LOG_DEBUG2, "emitter: no handler set; ignoring");
            return;
        }
        do_data(data);
    }

    void emit_flush() override {
        logger->log(MK_LOG_DEBUG2, "emitter: emit 'flush' event");
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
            return;
        }
        if (!do_flush) {
            logger->log(MK_LOG_DEBUG2, "emitter: no handler set; ignoring");
            return;
        }
        do_flush();
    }

    void emit_error(Error err) override {
        logger->log(MK_LOG_DEBUG2, "emitter: emit 'error' event "
                    "(error = '%s')", err.explain().c_str());
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
            return;
        }
        if (!do_error) {
            logger->log(MK_LOG_DEBUG2, "emitter: no handler set; ignoring");
            return;
        }
        do_error(err);
    }

    void on_connect(std::function<void()> fn) override {
        logger->log(MK_LOG_DEBUG2, "emitter: %sregister 'connect' handler",
                    (fn != nullptr) ? "" : "un");
        do_connect = fn;
    }

    void on_data(std::function<void(Buffer)> fn) override {
        logger->log(MK_LOG_DEBUG2, "emitter: %sregister 'data' handler",
                    (fn != nullptr) ? "" : "un");
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
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
        logger->log(MK_LOG_DEBUG2, "emitter: %sregister 'flush' handler",
                    (fn != nullptr) ? "" : "un");
        do_flush = fn;
    }

    void on_error(std::function<void(Error)> fn) override {
        logger->log(MK_LOG_DEBUG2, "emitter: %sregister 'error' handler",
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
        logger->log(MK_LOG_DEBUG2, "emitter: send opaque data");
        if (p == nullptr) {
            throw std::runtime_error("null pointer");
        }
        write(Buffer(p, n));
    }

    void write(std::string s) override {
        logger->log(MK_LOG_DEBUG2, "emitter: send string");
        write(Buffer(s));
    }

    void write(Buffer data) override {
        logger->log(MK_LOG_DEBUG2, "emitter: send buffer");
        if (do_record_sent_data) {
            sent_data_record.write(data.peek());
        }
        output_buff << data;
        if (close_pending) {
            logger->log(MK_LOG_DEBUG2, "emitter: already closed; ignoring");
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

    // Protected methods of TransportPollable: not implemented

  protected:
    // TODO: it would probably better to have accessors
    Var<Reactor> reactor = Reactor::global();
    Var<Logger> logger = Logger::global();
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
};

class Emitter : public EmitterBase {
  public:
    Emitter(Var<Reactor> reactor, Var<Logger> logger)
        : EmitterBase(reactor, logger) {}

    ~Emitter() override;

  protected:
    void adjust_timeout(double timeo) override {
        logger->log(MK_LOG_DEBUG2, "emitter: adjust_timeout %f", timeo);
    }

    void shutdown() override {}

    void start_reading() override {}
    void stop_reading() override {}
    void start_writing() override {}
};

} // namespace net
} // namespace mk
#endif
