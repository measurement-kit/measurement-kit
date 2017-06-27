// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/ndt/run_impl.hpp"

namespace mk {
namespace ndt {

void run_with_specific_server(Var<Entry> entry, std::string address, int port,
                              Callback<Error> callback, Settings settings,
                              Var<Reactor> reactor, Var<Logger> logger) {
    run_with_specific_server_impl<
        protocol::connect, protocol::send_extended_login,
        protocol::recv_and_ignore_kickoff, protocol::wait_in_queue,
        protocol::recv_version, protocol::recv_tests_id, protocol::run_tests,
        protocol::recv_results_and_logout, protocol::wait_close,
        protocol::disconnect_and_callback>(entry, address, port, callback, settings,
                                           reactor, logger);
}

void run(Var<Entry> entry, Callback<Error> callback, Settings settings,
         Var<Reactor> reactor, Var<Logger> logger) {
    run_impl(entry, callback, settings, reactor, logger);
}

} // namespace mk
} // namespace ndt
