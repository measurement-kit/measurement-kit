// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_PROTOCOL_HPP
#define SRC_NDT_PROTOCOL_HPP

#include "src/common/utils.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/messages.hpp"
#include "src/ndt/test_c2s.hpp"
#include "src/ndt/test_s2c.hpp"
#include "src/ndt/test_meta.hpp"
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {
namespace protocol {

using namespace net;

void connect(Var<Context> ctx, Callback<Error> callback);
void send_extended_login(Var<Context> ctx, Callback<Error> callback);
void recv_and_ignore_kickoff(Var<Context> ctx, Callback<Error> callback);
void wait_in_queue(Var<Context> ctx, Callback<Error> callback);
void recv_version(Var<Context> ctx, Callback<Error> callback);
void recv_tests_id(Var<Context> ctx, Callback<Error> callback);
void run_tests(Var<Context> ctx, Callback<Error> callback);
void recv_results_and_logout(Var<Context> ctx, Callback<Error> callback);
void wait_close(Var<Context> ctx, Callback<Error> callback);
void disconnect_and_callback(Var<Context> ctx, Error err);

} // namespace protocol
} // namespace mk
} // namespace ndt
#endif
