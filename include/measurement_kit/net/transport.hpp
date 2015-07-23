// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_NET_TRANSPORT_HPP
#define MEASUREMENT_KIT_NET_TRANSPORT_HPP

//
// Generic transport (stream socket, SOCKSv5, etc.)
//

#include <functional>
#include <string>

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/log.hpp>
#include <measurement_kit/common/pointer.hpp>
#include <measurement_kit/common/settings.hpp>

#include <measurement_kit/net/buffer.hpp>

namespace measurement_kit {
namespace net {

using namespace measurement_kit::common;

struct Transport : public NonMovable, public NonCopyable {

    virtual void emit_connect() = 0;

    virtual void emit_data(SharedPointer<Buffer>) = 0;

    virtual void emit_flush() = 0;

    virtual void emit_error(Error) = 0;

    Transport() {}

    virtual void on_connect(std::function<void()>) = 0;

    virtual void on_ssl(std::function<void()>) = 0;

    virtual void on_data(std::function<void(SharedPointer<Buffer>)>) = 0;

    virtual void on_flush(std::function<void()>) = 0;

    virtual void on_error(std::function<void(Error)>) = 0;

    virtual void set_timeout(double) = 0;

    virtual void clear_timeout() = 0;

    virtual void send(const void*, size_t) = 0;

    virtual void send(std::string) = 0;

    virtual void send(SharedPointer<Buffer>) = 0;

    virtual void send(Buffer&) = 0;

    virtual void close() = 0;

    virtual std::string socks5_address() = 0;

    virtual std::string socks5_port() = 0;
};

SharedPointer<Transport>
connect(Settings, SharedPointer<Logger> = DefaultLogger::get());

}}
#endif
