// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

TcpConnect::TcpConnect() : TcpConnect("", {}) {}

TcpConnect::TcpConnect(std::string f, Settings o) : NetTest(f, o) {
    test_name = "tcp_connect";
    test_version = "0.0.1";
    needs_input = true;
}

void TcpConnect::main(std::string input, Settings options,
                      Callback<report::Entry> cb) {
    ooni::tcp_connect(input, options, [=](Var<report::Entry> entry) {
        cb(*entry);
    }, reactor, logger);
}

Var<NetTest> TcpConnect::create_test_() {
    TcpConnect *test = new TcpConnect(input_filepath, options);
    test->logger = logger;
    test->reactor = reactor;
    test->output_filepath = output_filepath;
    test->entry_cb = entry_cb;
    test->begin_cb = begin_cb;
    test->end_cb = end_cb;
    return Var<NetTest>(test);
}

} // namespace nettests
} // namespace mk
