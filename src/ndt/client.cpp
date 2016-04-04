// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/client.hpp"
#include "src/ndt/protocol.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {

void client(std::string address, int port, Callback<> callback,
            Settings settings, Logger *logger, Poller *poller) {
    client_impl<protocol::connect, protocol::send_extended_login,
                protocol::recv_and_ignore_kickoff, protocol::wait_in_queue,
                protocol::recv_version, protocol::recv_tests_id,
                protocol::run_tests, protocol::recv_results_and_logout,
                protocol::wait_close, protocol::disconnect_and_callback>(
        address, port, callback, settings, logger, poller);
}

} // namespace mk
} // namespace ndt
