// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

TcpConnectTest::TcpConnectTest() : BaseTest() {
    runnable.reset(new TcpConnectRunnable);
    runnable->test_name = "tcp_connect";
    runnable->test_version = "0.1.0";
    runnable->needs_input = true;
}

void TcpConnectRunnable::main(std::string input, Settings options,
                              Callback<SharedPtr<report::Entry>> cb) {
    ooni::tcp_connect(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
