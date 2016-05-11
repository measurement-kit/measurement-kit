// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_MESSAGES_IMPL_HPP
#define SRC_NDT_MESSAGES_IMPL_HPP

#include "src/ndt/messages.hpp"
#include "src/ext/json/src/json.hpp"
#include <measurement_kit/net.hpp>

namespace mk {
namespace ndt {
namespace messages {

using namespace mk::net;
using json = nlohmann::json;

template <decltype(net::readn) readn = net::readn>
void read_ndt_impl(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback) {

    // Receive message type (1 byte) and length (2 bytes)
    readn(ctx->txp, ctx->buff, 3, [=](Error err) {
        if (err) {
            callback(err, 0, "");
            return;
        }
        ErrorOr<uint8_t> type = ctx->buff->read_uint8();
        if (!type) {
            callback(type.as_error(), 0, "");
            return;
        }
        ErrorOr<uint16_t> length = ctx->buff->read_uint16();
        if (!length) {
            callback(length.as_error(), 0, "");
            return;
        }

        // Now read the message payload (`length` bytes in total)
        readn(ctx->txp, ctx->buff, *length, [=](Error err) {
            if (err) {
                callback(err, 0, "");
                return;
            }
            std::string s = ctx->buff->readn(*length);
            debug("< [%d]: (%d) %s", *length, *type, s.c_str());
            callback(NoError(), *type, s);
        });
    });
}

template <decltype(read_ndt) read_ndt = read_ndt>
void read_json_impl(Var<Context> ctx, Callback<Error, uint8_t, json> callback) {
    read_ndt(ctx, [=](Error err, uint8_t type, std::string m) {
        json message;
        if (err) {
            callback(err, 0, message);
            return;
        }
        try {
            message = json::parse(m);
        } catch (const std::exception &) {
            callback(GenericError(), 0, message);
            return;
        }
        callback(NoError(), type, message);
    });
}

template<decltype(read_json) read_json = read_json>
void read_impl(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback) {
    read_json(ctx, [=](Error error, uint8_t type, json message) {
        if (error) {
            callback(error, 0, "");
            return;
        }
        std::string s;
        try {
            s = message["msg"];
        } catch (const std::exception &) {
            callback(GenericError(), 0, "");
            return;
        }
        callback(NoError(), type, s);
    });
}

} // namespace messages
} // namespace ndt
} // namespace mk
#endif
