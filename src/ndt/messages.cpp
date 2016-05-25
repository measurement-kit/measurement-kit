// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/messages_impl.hpp"

namespace mk {
namespace ndt {
namespace messages {

void read_ll(Var<Context> ctx,
             mk::Callback<Error, uint8_t, std::string> callback,
             Var<Reactor> reactor) {
    read_ll_impl(ctx, callback, reactor);
}

void read_json(Var<Context> ctx, Callback<Error, uint8_t, json> callback,
               Var<Reactor> reactor) {
    read_json_impl(ctx, callback, reactor);
}

void read_msg(Var<Context> ctx, Callback<Error, uint8_t, std::string> cb,
              Var<Reactor> reactor) {
    read_msg_impl(ctx, cb, reactor);
}

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests) {
    return format_any(MSG_EXTENDED_LOGIN,
                      json{
                          {"msg", MSG_NDT_VERSION},
                          {"tests", lexical_cast<std::string>((int)tests)},
                      });
}

ErrorOr<Buffer> format_test_msg(std::string s) {
    return format_any(TEST_MSG, json{
                                    {"msg", s},
                                });
}

void write(Var<Context> ctx, Buffer buff, Callback<Error> cb) {
    std::string s = buff.peek();
    ctx->logger->debug("> [%lu]: (%d) %s", s.length(), s.c_str()[0],
                       s.substr(3).c_str());
    net::write(ctx->txp, buff, cb);
}

void write_noasync(Var<Context> ctx, Buffer buff) {
    std::string s = buff.peek();
    ctx->logger->debug("> [%lu]: (%d) %s", s.length(), s.c_str()[0],
                       s.substr(3).c_str());
    ctx->txp->write(buff);
}

} // namespace messages
} // namespace ndt
} // namespace mk
