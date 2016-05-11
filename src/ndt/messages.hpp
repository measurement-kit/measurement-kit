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

void read_ndt(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback);
void read_json(Var<Context> ctx, Callback<Error, uint8_t, json> callback);
void read(Var<Context> ctx, Callback<Error, uint8_t, std::string> callback);

ErrorOr<Buffer> format_msg_extended_login(unsigned char tests);
ErrorOr<Buffer> format_test_msg(std::string s);

void write(Var<Context>, Buffer, Callback<Error>);
void write_noasync(Var<Context>, Buffer);

} // namespace messages
} // namespace ndt
} // namespace mk
#endif
