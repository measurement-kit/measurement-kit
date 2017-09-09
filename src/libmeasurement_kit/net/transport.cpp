// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <cassert>
#include <measurement_kit/net.hpp>
#include "private/net/connect.hpp"
#include "private/net/emitter.hpp"
#include "private/net/socks5.hpp"

namespace mk {
namespace net {

TransportEmitter::~TransportEmitter() {}
TransportRecorder::~TransportRecorder() {}
TransportWriter::~TransportWriter() {}
TransportSocks5::~TransportSocks5() {}
TransportPollable::~TransportPollable() {}
TransportConnectable::~TransportConnectable() {}
TransportSockNamePeerName::~TransportSockNamePeerName() {}
Transport::~Transport() {}

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb) {
    txp->on_flush([=]() {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        cb(NoError());
    });
    txp->on_error([=](Error err) {
        txp->on_flush(nullptr);
        txp->on_error(nullptr);
        cb(err);
    });
    txp->write(buf);
}

void readn(Var<Transport> txp, Var<Buffer> buff, size_t n, Callback<Error> cb,
           Var<Reactor> reactor) {
    if (buff->length() >= n) {
        // Shortcut that simplifies coding a great deal - yet, do not callback
        // immediately to avoid O(N) stack consumption
        reactor->call_soon([=]() {
            cb(NoError());
        });
        return;
    }
    txp->on_data([=](Buffer d) {
        *buff << d;
        if (buff->length() < n) {
            return;
        }
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        cb(NoError());
    });
    txp->on_error([=](Error error) {
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        cb(error);
    });
}

void read(Var<Transport> t, Var<Buffer> buff, Callback<Error> callback,
          Var<Reactor> reactor) {
    readn(t, buff, 1, callback, reactor);
}

void continue_reading(
    Var<Transport> txp,
    Callback<Error, Buffer, std::function<void()> &> callback) {
    txp->on_data([=](Buffer data) {
        std::function<void()> canceller{[=]() {
            txp->on_data(nullptr);
            txp->on_error(nullptr);
        }};
        callback(NoError(), data, canceller);
    });
    txp->on_error([=](Error error) {
        txp->on_data(nullptr);
        txp->on_error(nullptr);
        std::function<void()> canceller{[=]() {}};
        callback(error, Buffer{}, canceller);
    });
}

} // namespace net
} // namespace mk
