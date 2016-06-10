// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <cassert>
#include <measurement_kit/net.hpp>
#include "src/net/connect.hpp"
#include "src/net/connection.hpp"
#include "src/net/emitter.hpp"
#include "src/net/socks5.hpp"
#include "src/net/ssl-context.hpp"

namespace mk {
namespace net {

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

} // namespace net
} // namespace mk
