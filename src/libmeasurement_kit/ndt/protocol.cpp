// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ndt/protocol_impl.hpp"

namespace mk {
namespace ndt {
namespace protocol {

void connect(SharedPtr<Context> ctx, Callback<Error> callback) {
    connect_impl(ctx, callback);
}

void send_extended_login(SharedPtr<Context> ctx, Callback<Error> callback) {
    send_extended_login_impl(ctx, callback);
}

void recv_and_ignore_kickoff(SharedPtr<Context> ctx, Callback<Error> callback) {
    recv_and_ignore_kickoff_impl(ctx, callback);
}

void wait_in_queue(SharedPtr<Context> ctx, Callback<Error> callback) {
    wait_in_queue_impl(ctx, callback);
}

void recv_version(SharedPtr<Context> ctx, Callback<Error> callback) {
    recv_version_impl(ctx, callback);
}

void recv_tests_id(SharedPtr<Context> ctx, Callback<Error> callback) {
    recv_tests_id_impl(ctx, callback);
}

void run_tests(SharedPtr<Context> ctx, Callback<Error> callback) {
    run_tests_impl(ctx, callback);
}

void recv_results_and_logout(SharedPtr<Context> ctx, Callback<Error> callback) {
    recv_results_and_logout_impl(ctx, callback);
}

void wait_close(SharedPtr<Context> ctx, Callback<Error> callback) {
    wait_close_impl(ctx, callback);
}

void disconnect_and_callback(SharedPtr<Context> ctx, Error err) {
    disconnect_and_callback_impl(ctx, err);
}

} // namespace protocol
} // namespace ndt
} // namespace mk
