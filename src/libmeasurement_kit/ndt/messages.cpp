// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ndt/messages_impl.hpp"

#include <sstream>

namespace mk {
namespace ndt {
namespace messages {

void read_ll(SharedPtr<Context> ctx,
             mk::Callback<Error, uint8_t, std::string> callback,
             SharedPtr<Reactor> reactor) {
    read_ll_impl(ctx, callback, reactor);
}

void read_json(SharedPtr<Context> ctx, Callback<Error, uint8_t, nlohmann::json> callback,
               SharedPtr<Reactor> reactor) {
    read_json_impl(ctx, callback, reactor);
}

void read_msg(SharedPtr<Context> ctx, Callback<Error, uint8_t, std::string> cb,
              SharedPtr<Reactor> reactor) {
    read_msg_impl(ctx, cb, reactor);
}

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests) {
    return format_any(MSG_EXTENDED_LOGIN, nlohmann::json{
                          {"msg", MSG_NDT_VERSION},
                          {"tests", std::to_string((int)tests)},
                      });
}

ErrorOr<Buffer> format_test_msg(std::string s) {
    return format_any(TEST_MSG, nlohmann::json{
                          {"msg", s},
                      });
}

ErrorOr<Buffer> format_msg_waiting() {
    return format_any(MSG_WAITING, nlohmann::json{
                          {"msg", ""},
                      });
}

void write(SharedPtr<Context> ctx, Buffer buff, Callback<Error> cb) {
    // Need to use string stream because the original printf() is not
    // guaranteed to work on old versions of Windows and with Mingw we
    // link against MSVCRT.DLL (Visual Studio 6.0; 1998). Thus we may
    // run on systems whose printf() doesn't understand `%zu`.
    //
    // TODO(bassosimone): find a better fix. Most likely that would be
    // to use C++ style logging for the logger.
    if (ctx->logger->get_verbosity() >= MK_LOG_DEBUG) {
        std::string s = buff.peek();
        std::stringstream ss;
        ss << ">[" << s.length() << "]: (" << s.c_str()[0] << ") "
           << s.substr(3).c_str();
        // It sucks that we format the message twice but this is only
        // for debugging, so it actually doesn't matter.
        ctx->logger->debug("%s", ss.str().c_str());
    }
    net::write(ctx->txp, buff, cb);
}

void write_noasync(SharedPtr<Context> ctx, Buffer buff) {
    // See above. TODO(bassosimone): find a better fix.
    if (ctx->logger->get_verbosity() >= MK_LOG_DEBUG) {
        std::string s = buff.peek();
        std::stringstream ss;
        ss << ">[" << s.length() << "]: (" << s.c_str()[0] << ") "
           << s.substr(3).c_str();
        ctx->logger->debug("%s", ss.str().c_str());
    }
    ctx->txp->write(buff);
}

Error add_to_report(SharedPtr<nlohmann::json> entry, std::string key, std::string item) {
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
