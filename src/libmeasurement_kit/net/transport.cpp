// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <cassert>
#include "src/libmeasurement_kit/net/transport.hpp"
#include "src/libmeasurement_kit/net/connect.hpp"
#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/net/socks5.hpp"

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

void write(SharedPtr<Transport> txp, Buffer buf, Callback<Error> cb) {
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

void readn(SharedPtr<Transport> txp, SharedPtr<Buffer> buff, size_t n, Callback<Error> cb,
           SharedPtr<Reactor> reactor) {
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

void read(SharedPtr<Transport> t, SharedPtr<Buffer> buff, Callback<Error> callback,
          SharedPtr<Reactor> reactor) {
    readn(t, buff, 1, callback, reactor);
}

} // namespace net
} // namespace mk
