// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_MESSAGES_HPP
#define SRC_NDT_MESSAGES_HPP

#include "src/ndt/context.hpp"
#include "src/ext/json/src/json.hpp"
#include <measurement_kit/net.hpp>

namespace mk {
namespace ndt {
namespace messages {

using namespace mk::net;
using json = nlohmann::json;

/// Receive message serialized according to the NDT protocol
void read_ndt(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback);

/// Testable implementation of read_ndt()
template <decltype(net::readn) readn = net::readn>
void read_ndt_impl(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback) {

    // Receive message type (1 byte) and length (2 bytes)
    readn(ctx->conn, ctx->buff, 3, [=](Error err) {
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
        readn(ctx->conn, ctx->buff, *length, [=](Error err) {
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

/// Receive message serialized according to NDT protocol and containing a JSON
void read_json(Var<Context> ctx, Callback<Error, uint8_t, json> callback);

/// Testable implementation of read_json()
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

/// Read JSON message, parse JSON, and return msg field
void read(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback);

/// Testable implementation of read()
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

/// Return formatted MSG_EXTENDED_LOGIN message or error on failure
ErrorOr<Buffer> format_msg_extended_login(unsigned char tests);

/// Return formatted TEST_MSG message
ErrorOr<Buffer> format_test_msg(std::string s);

/// Wrapper for net::write()
void write(Var<Context>, Buffer, Callback<Error>);

/// Wrapper for ctx->conn->write()
void write_noasync(Var<Context>, Buffer);

} // namespace messages
} // namespace ndt
} // namespace mk
#endif
