// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

Var<NetTest> DnsInjection::create_test_() {
    DnsInjection *test = new DnsInjection(input_filepath, options);
    test->logger = logger;
    test->reactor = reactor;
    test->output_filepath = output_filepath;
    return Var<NetTest>(test);
}

Var<NetTest> HttpInvalidRequestLine::create_test_() {
    HttpInvalidRequestLine *test = new HttpInvalidRequestLine(options);
    test->logger = logger;
    test->reactor = reactor;
    test->output_filepath = output_filepath;
    return Var<NetTest>(test);
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
