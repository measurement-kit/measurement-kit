// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

namespace mk {
namespace nettests {

TcpConnectRunnable::TcpConnectRunnable() noexcept {
    test_name = "tcp_connect";
    test_version = "0.1.0";
    needs_input = true;
}

void TcpConnectRunnable::main(std::string input, Settings options,
                              Callback<SharedPtr<nlohmann::json>> cb) {
    ooni::tcp_connect(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
