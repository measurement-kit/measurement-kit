// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void tcp_connect(std::string input, Settings options,
                 Callback<Var<Entry>> callback, Var<Reactor> reactor,
                 Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    (*entry)["connection"] = nullptr;
    options["host"] = input;
    templates::tcp_connect(options, [=](Error err, Var<net::Transport> txp) {
        logger->debug("tcp_connect: Got response to TCP connect test");
        if (err) {
            (*entry)["connection"] = err.as_ooni_error();
            callback(entry);
            return;
        }
        txp->close([=]() {
            (*entry)["connection"] = "success";
            callback(entry);
        });
    }, reactor, logger);
}

Var<NetTest> TcpConnect::create_test_() {
    TcpConnect *test = new TcpConnect(input_filepath, options);
    test->logger = logger;
    test->reactor = reactor;
    test->output_filepath = output_filepath;
    return Var<NetTest>(test);
}

} // namespace ooni
} // namespace mk
