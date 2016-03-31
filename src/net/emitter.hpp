// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_EMITTER_HPP
#define SRC_NET_EMITTER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

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

    Emitter(Logger *lp = Logger::global()) : logger(lp) {}

    ~Emitter() override;

    void on_connect(std::function<void()> fn) override {
        logger->debug("emitter: register 'connect' handler");
        do_connect = fn;
    }

    void on_data(std::function<void(Buffer)> fn) override {
        logger->debug("emitter: register 'data' handler");
        do_data = fn;
    }

    void on_flush(std::function<void()> fn) override {
        logger->debug("emitter: register 'flush' handler");
        do_flush = fn;
    }

    void on_error(std::function<void(Error)> fn) override {
        logger->debug("emitter: register 'error' handler");
        do_error = fn;
    }

    void set_timeout(double timeo) override {
        logger->debug("emitter: set_timeout %f", timeo);
    }

    void clear_timeout() override { logger->debug("emitter: clear_timeout"); }

    void send(const void *, size_t) override {
        logger->debug("emitter: send opaque data");
    }

    void send(std::string) override { logger->debug("emitter: send string"); }

    void send(Buffer) override { logger->debug("emitter: send buffer"); }

    void close() override { logger->debug("emitter: close"); }

    std::string socks5_address() override { return ""; }

    std::string socks5_port() override { return ""; }

  protected:
    Logger *logger = Logger::global();

  private:
    SafelyOverridableFunc<void()> do_connect = []() {};
    SafelyOverridableFunc<void(Buffer)> do_data = [](Buffer) {};
    SafelyOverridableFunc<void()> do_flush = []() {};
    SafelyOverridableFunc<void(Error)> do_error = [](Error) {};
};

} // namespace net
} // namespace mk
#endif
