// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_EMITTER_HPP
#define SRC_NET_EMITTER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <stdexcept>

namespace mk {
namespace net {

class Emitter : public Transport {
  public:
    void emit_connect() override {
        logger->debug("emitter: emit 'connect' event");
        do_connect();
    }

    void emit_data(Buffer data) override {
        logger->debug("emitter: emit 'data' event");
        if (do_record_received_data) {
            received_data_record.write(data.peek());
        }
        do_data(data);
    }

    void emit_flush() override {
        logger->debug("emitter: emit 'flush' event");
        do_flush();
    }

    void emit_error(Error err) override {
        logger->debug("emitter: emit 'error' event");
        do_error(err);
    }

    Emitter(Var<Logger> lp = Logger::global()) : logger(lp) {}

    ~Emitter() override;

    void on_connect(std::function<void()> fn) override {
        logger->debug("emitter: register 'connect' handler");
        do_connect = fn;
    }

    void on_data(std::function<void(Buffer)> fn) override {
        logger->debug("emitter: register 'data' handler");
        if (fn) {
            enable_read();
        } else {
            disable_read();
        }
        do_data = fn;
    }

    virtual void enable_read() {}
    virtual void disable_read() {}

    void on_flush(std::function<void()> fn) override {
        logger->debug("emitter: register 'flush' handler");
        do_flush = fn;
    }

    void on_error(std::function<void(Error)> fn) override {
        logger->debug("emitter: register 'error' handler");
        do_error = fn;
    }

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

    void set_timeout(double timeo) override {
        logger->debug("emitter: set_timeout %f", timeo);
    }

    void clear_timeout() override { logger->debug("emitter: clear_timeout"); }

    void write(const void *p, size_t n) override {
        logger->debug("emitter: send opaque data");
        if (p == nullptr) {
            throw std::runtime_error("null pointer");
        }
        write(Buffer(p, n));
    }

    void write(std::string s) override {
        logger->debug("emitter: send string");
        write(Buffer(s));
    }

    void write(Buffer data) override {
        logger->debug("emitter: send buffer");
        if (do_record_sent_data) {
            sent_data_record.write(data.peek());
        }
        do_send(data);
    }

    // Implements actual send and should be override by subclasses
    virtual void do_send(Buffer) {}

    void close(std::function<void()>) override {}

    std::string socks5_address() override { return ""; }

    std::string socks5_port() override { return ""; }

  protected:
    Var<Logger> logger = Logger::global();

  private:
    Delegate<void()> do_connect = []() {};
    Delegate<void(Buffer)> do_data = [](Buffer) {};
    Delegate<void()> do_flush = []() {};
    Delegate<void(Error)> do_error = [](Error) {};
    bool do_record_received_data = false;
    Buffer received_data_record;
    bool do_record_sent_data = false;
    Buffer sent_data_record;
};

} // namespace net
} // namespace mk
#endif
