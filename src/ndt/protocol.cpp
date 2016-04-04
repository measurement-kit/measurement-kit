// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/protocol.hpp"

namespace mk {
namespace ndt {
namespace protocol {

void connect(Var<Context> ctx, Callback<> callback) {
    connect_impl(ctx, callback);
}

void send_extended_login(Var<Context> ctx, Callback<> callback) {
    send_extended_login_impl(ctx, callback);
}

void recv_and_ignore_kickoff(Var<Context> ctx, Callback<> callback) {
    recv_and_ignore_kickoff_impl(ctx, callback);
}

void wait_in_queue(Var<Context> ctx, Callback<> callback) {
    wait_in_queue_impl(ctx, callback);
}

void recv_version(Var<Context> ctx, Callback<> callback) {
    recv_version_impl(ctx, callback);
}

void recv_tests_id(Var<Context> ctx, Callback<> callback) {
    recv_tests_id_impl(ctx, callback);
}

void run_tests(Var<Context> ctx, Callback<> callback) {
    run_tests_impl(ctx, callback);
}

void recv_results_and_logout(Var<Context> ctx, Callback<> callback) {
    recv_results_and_logout_impl(ctx, callback);
}

void wait_close(Var<Context> ctx, Callback<> callback) {
    wait_close_impl(ctx, callback);
}

void disconnect_and_callback(Var<Context> ctx, Error err) {
    if (ctx->conn) {
        Var<Transport> conn = ctx->conn;
        ctx->conn = nullptr;
        conn->close([=]() { ctx->callback(err); });
        return;
    }
    ctx->callback(err);
}

} // namespace protocol
} // namespace ndt
} // namespace mk
