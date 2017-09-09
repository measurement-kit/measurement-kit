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

// TODO: here we should probably also make sure that one cannot register
// a new pending operation if a previous one is already in progress.

void write(Var<Transport> txp, Buffer buf, Callback<Error> cb) {
    txp->on_flush([=](Error err) {
        txp->on_flush(nullptr);
        cb(err);
    });
    txp->write(buf);
}

void continue_writing(
    Var<Transport> txp,
    Callback<Error, std::function<void()> &> callback) {
    txp->on_flush([=](Error err) {
        std::function<void()> canceller{[=]() { txp->on_flush(nullptr); }};
        if (!!err) {
            canceller();
        }
        callback(err, canceller);
    });
}

void readn_into(Var<Transport> txp, Var<Buffer> buff, size_t n,
                Callback<Error> cb) {
    if (buff->length() >= n) {
        // Shortcut that simplifies coding a great deal - yet, do not callback
        // immediately to avoid O(N) stack consumption
        txp->get_reactor()->call_soon([=]() { cb(NoError()); });
        return;
    }
    readn(txp, n, [=](Error err, Buffer data) {
        *buff << data;
        cb(err);
    });
}

void readn(Var<Transport> txp, size_t n, Callback<Error, Buffer> callback) {
    Var<Buffer> buff = Buffer::make();
    continue_reading(
        txp, [=](Error err, Buffer data, std::function<void()> &cancel) {
            *buff << data;
            if (buff->length() < n && !err) {
                return;
            }
            cancel();
            callback(err, *buff);
        });
}

void read_into(Var<Transport> txp, Var<Buffer> buff, Callback<Error> cb) {
    read(txp, [=](Error err, Buffer data) {
        *buff << data;
        cb(err);
    });
}

void read(Var<Transport> txp, Callback<Error, Buffer> callback) {
    continue_reading(
        txp, [=](Error err, Buffer data, std::function<void()> &cancel) {
            cancel();
            callback(err, data);
        });
}

void continue_reading_into(
    Var<Transport> txp, Var<Buffer> buff,
    Callback<Error, std::function<void()> &> callback) {
    continue_reading(
        txp, [=](Error err, Buffer data, std::function<void()> &cancel) {
            *buff << data;
            callback(err, cancel);
        });
}

void continue_reading(
    Var<Transport> txp,
    Callback<Error, Buffer, std::function<void()> &> callback) {
    txp->on_data([=](Error err, Buffer data) {
        std::function<void()> canceller{[=]() { txp->on_data(nullptr); }};
        if (!!err) {
            canceller();
        }
        callback(err, data, canceller);
    });
}

} // namespace net
} // namespace mk
