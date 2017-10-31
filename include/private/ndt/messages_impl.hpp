// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NDT_MESSAGES_IMPL_HPP
#define PRIVATE_NDT_MESSAGES_IMPL_HPP

/*
 * To have UINT16_MAX defined. Required on Android API < 21.
 *
 * See <http://stackoverflow.com/a/986584>.
 */
#define __STDC_LIMIT_MACROS

#include "private/common/mock.hpp"

#include "../ndt/internal.hpp"
#include <cassert>

namespace mk {
namespace ndt {
namespace messages {

/*
    +----------+------------+-------------------+
    | type (1) | length (2) | payload (0-65535) |
    +----------+------------+-------------------+
*/
template <MK_MOCK_AS(net::readn, net_readn_first),
          MK_MOCK_AS(net::readn, net_readn_second)>
void read_ll_impl(SharedPtr<Context> ctx,
                  Callback<Error, uint8_t, std::string> callback,
                  SharedPtr<Reactor> reactor) {

    // Receive message type (1 byte) and length (2 bytes)
    net_readn_first(ctx->txp, ctx->buff, 3, [=](Error err) {
        if (err) {
            callback(ReadingMessageTypeLengthError(err), 0, "");
            return;
        }
        ErrorOr<uint8_t> type = ctx->buff->read_uint8();
        ErrorOr<uint16_t> length = ctx->buff->read_uint16();
        // Note: we don't check for `type` and `length` true-ness because
        // we are after a readn() which should ensure enough space is there,
        // hence, if that's not the case, we'll see an exception

        // Now read the message payload (`*length` bytes in total)
        net_readn_second(ctx->txp, ctx->buff, *length, [=](Error err) {
            if (err) {
                callback(ReadingMessagePayloadError(err), 0, "");
                return;
            }
            std::string s = ctx->buff->readn(*length);
            // TODO: rather than using assert() here we should modify readn()
            // to return ErrorOr<> (exceptions are better than asserts)
            assert(s.size() == *length);
            ctx->logger->debug("< [%d]: (%d) %s", *length, *type, s.c_str());
            callback(NoError(), *type, s);
        }, reactor);
    }, reactor);
}

// Like `read_ll()` but decode the payload using JSON
template <MK_MOCK(read_ll)>
void read_json_impl(SharedPtr<Context> ctx, Callback<Error, uint8_t, Json> callback,
                    SharedPtr<Reactor> reactor) {
    read_ll(ctx, [=](Error err, uint8_t type, std::string m) {
        Json message;
        if (err) {
            callback(err, 0, message);
            return;
        }
        try {
            message = Json::parse(m);
        } catch (const std::invalid_argument &) {
            callback(JsonParseError(), 0, message);
            return;
        }
        callback(NoError(), type, message);
    }, reactor);
}

// Like `read_json()` but return the `msg` field only
template <MK_MOCK(read_json)>
void read_msg_impl(SharedPtr<Context> ctx,
                   Callback<Error, uint8_t, std::string> callback,
                   SharedPtr<Reactor> reactor) {
    read_json(ctx, [=](Error error, uint8_t type, Json message) {
        if (error) {
            callback(error, 0, "");
            return;
        }
        std::string s;
        try {
            s = message.at("msg");
        } catch (const std::out_of_range &) {
            callback(JsonKeyError(), 0, "");
            return;
        } catch (const std::domain_error &) {
            callback(JsonDomainError(), 0, "");
            return;
        }
        callback(NoError(), type, s);
    }, reactor);
}

static inline ErrorOr<Buffer> format_any(unsigned char type, Json message) {
    Buffer out;
    out.write_uint8(type);
    std::string s = message.dump();
    if (s.size() > UINT16_MAX) {
        return MessageTooLongError();
    }
    // Cast safe because we've just excluded the case where it's bigger
    uint16_t length = (uint16_t)s.size();
    out.write_uint16(length);
    out.write(s.data(), s.size());
    return out;
}

} // namespace messages
} // namespace ndt
} // namespace mk
#endif
