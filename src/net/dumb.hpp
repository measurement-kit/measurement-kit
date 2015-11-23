// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_DUMB_HPP
#define MEASUREMENT_KIT_NET_DUMB_HPP

//
// Dumb transport
//

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/net/transport.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

class Dumb : public Transport {
  private:
    std::function<void()> do_connect = []() {};
    std::function<void(Buffer)> do_data = [](Buffer) {};
    std::function<void()> do_flush = []() {};
    std::function<void(Error)> do_error = [](Error) {};

  protected:
    Logger *logger = Logger::global();

  public:
    void emit_connect() override {
        logger->debug("dumb: emit 'connect' event");
        // With GNU C++ library, if a std::function sets itself, the
        // associated context is free() leading to segfault
        auto fn = do_connect;
        fn();
    }

    virtual void emit_data(Buffer data) override {
        logger->debug("dumb: emit 'data' event");
        // With GNU C++ library, if a std::function sets itself, the
        // associated context is free() leading to segfault
        auto fn = do_data;
        fn(data);
    }

    void emit_flush() override {
        logger->debug("dumb: emit 'flush' event");
        // With GNU C++ library, if a std::function sets itself, the
        // associated context is free() leading to segfault
        auto fn = do_flush;
        fn();
    }

    void emit_error(Error err) override {
        logger->debug("dumb: emit 'error' event");
        // With GNU C++ library, if a std::function sets itself, the
        // associated context is free() leading to segfault
        auto fn = do_error;
        fn(err);
    }

    Dumb(Logger *lp = Logger::global()) : logger(lp) {}

    ~Dumb() override {}

    void on_connect(std::function<void()> fn) override {
        logger->debug("dumb: register 'connect' handler");
        do_connect = fn;
    }

    virtual void on_data(std::function<void(Buffer)> fn) override {
        logger->debug("dumb: register 'data' handler");
        do_data = fn;
    }

    void on_flush(std::function<void()> fn) override {
        logger->debug("dumb: register 'flush' handler");
        do_flush = fn;
    }

    void on_error(std::function<void(Error)> fn) override {
        logger->debug("dumb: register 'error' handler");
        do_error = fn;
    }

    void set_timeout(double timeo) override {
        logger->debug("dumb: set_timeout %f", timeo);
    }

    void clear_timeout() override { logger->debug("dumb: clear_timeout"); }

    void send(const void *, size_t) override {
        logger->debug("dumb: send opaque data");
    }

    void send(std::string) override { logger->debug("dumb: send string"); }

    void send(Buffer) override { logger->debug("dumb: send buffer"); }

    void close() override { logger->debug("dumb: close"); }

    std::string socks5_address() override { return ""; }

    std::string socks5_port() override { return ""; }
};

} // namespace net
} // namespace measurement_kit
#endif
