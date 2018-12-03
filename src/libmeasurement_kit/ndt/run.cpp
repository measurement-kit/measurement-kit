// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ndt/run_impl.hpp"

namespace mk {
namespace ndt {

void run_with_specific_server(SharedPtr<nlohmann::json> entry, std::string address, int port,
                              Callback<Error> callback, Settings settings,
                              SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    run_with_specific_server_impl<
        protocol::connect, protocol::send_extended_login,
        protocol::recv_and_ignore_kickoff, protocol::wait_in_queue,
        protocol::recv_version, protocol::recv_tests_id, protocol::run_tests,
        protocol::recv_results_and_logout, protocol::wait_close,
        protocol::disconnect_and_callback>(entry, address, port, callback, settings,
                                           reactor, logger);
}

void run(SharedPtr<nlohmann::json> entry, Callback<Error> callback, Settings settings,
         SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    run_impl(entry, callback, settings, reactor, logger);
}

} // namespace mk
} // namespace ndt
