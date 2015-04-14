/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */
#ifndef IGHT_NET_DUMB_HPP
# define IGHT_NET_DUMB_HPP

//
// Dumb transport
//

#include <ight/common/log.hpp>
#include <ight/net/transport.hpp>

namespace ight {
namespace net {
namespace dumb {

using namespace ight::common::pointer;
using namespace ight::common::log;
using namespace ight::common::error;
using namespace ight::net::buffer;
using namespace ight::net::transport;

class Dumb : public Transport {

    std::function<void()> do_connect;
    std::function<void(SharedPointer<Buffer>)> do_data;
    std::function<void()> do_flush;
    std::function<void(Error)> do_error;

    SharedPointer<Logger> logger = DefaultLogger::get();

public:

    virtual void emit_connect() override {
        logger->debug("dumb: emit 'connect' event");
        do_connect();
    }

    virtual void emit_data(SharedPointer<Buffer> data) override {
        logger->debug("dumb: emit 'data' event");
        do_data(data);
    }

    virtual void emit_flush() override {
        logger->debug("dumb: emit 'flush' event");
        do_flush();
    }

    virtual void emit_error(Error err) override {
        logger->debug("dumb: emit 'error' event");
        do_error(err);
    }

    Dumb(SharedPointer<Logger> lp = DefaultLogger::get()) : logger(lp) {}

    virtual void on_connect(std::function<void()> fn) override {
        logger->debug("dumb: register 'connect' handler");
        do_connect = fn;
    }

    virtual void on_ssl(std::function<void()>) override {
        logger->debug("dumb: register 'ssl' handler");
        // currently not implemented
    }

    virtual void
    on_data(std::function<void(SharedPointer<Buffer>)> fn) override {
        logger->debug("dumb: register 'data' handler");
        do_data = fn;
    }

    virtual void on_flush(std::function<void()> fn) override {
        logger->debug("dumb: register 'flush' handler");
        do_flush = fn;
    }

    virtual void on_error(std::function<void(Error)> fn) override {
        logger->debug("dumb: register 'error' handler");
        do_error = fn;
    }

    virtual void set_timeout(double timeo) override {
        logger->debug("dumb: set_timeout %f", timeo);
    }

    virtual void clear_timeout() override {
        logger->debug("dumb: clear_timeout");
    }

    virtual void send(const void*, size_t) override {
        logger->debug("dumb: send opaque data");
    }

    virtual void send(std::string) override {
        logger->debug("dumb: send string");
    }

    virtual void send(SharedPointer<Buffer>) override {
        logger->debug("dumb: send pointer to buffer");
    }

    virtual void send(Buffer&) override {
        logger->debug("dumb: send buffer");
    }

    virtual void close() override {
        logger->debug("dumb: close");
    }

    virtual std::string socks5_address() override {
        return "";
    }

    virtual std::string socks5_port() override {
        return "";
    }
};

}}}
#endif
