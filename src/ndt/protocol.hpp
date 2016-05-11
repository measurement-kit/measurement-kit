// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_PROTOCOL_HPP
#define SRC_NDT_PROTOCOL_HPP

#include "src/common/utils.hpp"
#include "src/ext/json/src/json.hpp"
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
using json = nlohmann::json;

/// Connects to the ndt server
void connect(Var<Context> ctx, Callback<Error> callback);

/// Send EXTENDED_LOGIN message to server
void send_extended_login(Var<Context> ctx, Callback<Error> callback);

/// Receive and ignore the special kickoff message
void recv_and_ignore_kickoff(Var<Context> ctx, Callback<Error> callback);

/// Wait for our turn in queue for some time
void wait_in_queue(Var<Context> ctx, Callback<Error> callback);

/// Receive LOGIN message from server with the server's version
void recv_version(Var<Context> ctx, Callback<Error> callback);

/// Receive IDs of tests the server is willing to perform with us
void recv_tests_id(Var<Context> ctx, Callback<Error> callback);

void run_tests(Var<Context> ctx, Callback<Error> callback); ///< Run tests

/// Recv results message and then logout message
void recv_results_and_logout(Var<Context> ctx, Callback<Error> callback);

/// Wait for server to close the connection
void wait_close(Var<Context> ctx, Callback<Error> callback);

/// Close connection and invoke final callback
void disconnect_and_callback(Var<Context> ctx, Error err);

} // namespace protocol
} // namespace mk
} // namespace ndt
#endif
