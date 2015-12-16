// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni/tcp_connect_test.hpp>

#include "src/ooni/tcp_connect.hpp"

namespace mk {
namespace ooni {

void TCPConnect::main(std::string input, Settings options,
                      std::function<void(report::Entry)> &&cb) {
    options["host"] = input;
    have_entry = cb;
    client = connect(options, [this]() {
        logger.debug("tcp_connect: Got response to TCP connect test");
        have_entry(entry);
    });
}

Var<mk::NetTest> TcpConnectTest::create_test_() {
    OoniTest *test = new TCPConnect(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    return Var<mk::NetTest>(test);
}

} // namespace ooni
} // namespace mk
