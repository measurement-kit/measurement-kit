// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ndt/messages_impl.hpp"

namespace mk {
namespace ndt {
namespace messages {

void read_ll(SharedPtr<Context> ctx,
             mk::Callback<Error, uint8_t, std::string> callback,
             SharedPtr<Reactor> reactor) {
    read_ll_impl(ctx, callback, reactor);
}

void read_json(SharedPtr<Context> ctx, Callback<Error, uint8_t, json> callback,
               SharedPtr<Reactor> reactor) {
    read_json_impl(ctx, callback, reactor);
}

void read_msg(SharedPtr<Context> ctx, Callback<Error, uint8_t, std::string> cb,
              SharedPtr<Reactor> reactor) {
    read_msg_impl(ctx, cb, reactor);
}

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests) {
    return format_any(MSG_EXTENDED_LOGIN, json{
                          {"msg", MSG_NDT_VERSION},
                          {"tests", lexical_cast<std::string>((int)tests)},
                      });
}

ErrorOr<Buffer> format_test_msg(std::string s) {
    return format_any(TEST_MSG, json{
                          {"msg", s},
                      });
}

ErrorOr<Buffer> format_msg_waiting() {
    return format_any(MSG_WAITING, json{
                          {"msg", ""},
                      });
}

void write(SharedPtr<Context> ctx, Buffer buff, Callback<Error> cb) {
    std::string s = buff.peek();
    ctx->logger->debug("> [%zu]: (%d) %s", s.length(), s.c_str()[0],
                       s.substr(3).c_str());
    net::write(ctx->txp, buff, cb);
}

void write_noasync(SharedPtr<Context> ctx, Buffer buff) {
    std::string s = buff.peek();
    ctx->logger->debug("> [%zu]: (%d) %s", s.length(), s.c_str()[0],
                       s.substr(3).c_str());
    ctx->txp->write(buff);
}

Error add_to_report(SharedPtr<Entry> entry, std::string key, std::string item) {
    std::list<std::string> list = split(item, ":");
    if (list.size() != 2) {
        return GenericError(); /* XXX use more specific error */
    }
    // XXX: We should make sure that we remove leading and trailing whitespaces
    std::string variable = list.front();
    std::string value = list.back();
    ErrorOr<long> as_long = lexical_cast_noexcept<long>(value);
    if (!!as_long) {
        (*entry)[key][variable] = *as_long;
        return NoError();
    }
    ErrorOr<double> as_double = lexical_cast_noexcept<double>(value);
    if (!!as_double) {
        (*entry)[key][variable] = *as_double;
        return NoError();
    }
    (*entry)[key][variable] = value;
    return NoError();
}

} // namespace messages
} // namespace ndt
} // namespace mk
