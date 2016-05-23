// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/run_impl.hpp"
#include <measurement_kit/mlabns.hpp>

namespace mk {
namespace ndt {

void run_with_specific_server(std::string address, int port,
                              Callback<Error> callback, Settings settings,
                              Var<Logger> logger, Var<Reactor> reactor) {
    run_with_specific_server_impl<
        protocol::connect, protocol::send_extended_login,
        protocol::recv_and_ignore_kickoff, protocol::wait_in_queue,
        protocol::recv_version, protocol::recv_tests_id, protocol::run_tests,
        protocol::recv_results_and_logout, protocol::wait_close,
        protocol::disconnect_and_callback>(address, port, callback, settings,
                                           logger, reactor);
}

void run(Callback<Error> callback, Settings settings, Var<Logger> logger,
         Var<Reactor> reactor) {
    run_impl(callback, settings, logger, reactor);
}

} // namespace mk
} // namespace ndt
