// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>                              // for seconds
#include <functional>                          // for function
#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni/dns_injection_test.hpp>
#include <measurement_kit/ooni/http_invalid_request_line_test.hpp>
#include <measurement_kit/ooni/tcp_connect_test.hpp>
#include <ratio>                               // for ratio
#include <sys/stat.h>
#include <thread>                              // for sleep_for
#include "src/ooni/dns_injection_impl.hpp"
#include "src/ooni/http_invalid_request_line_impl.hpp"
#include "src/ooni/tcp_connect_impl.hpp"

namespace mk {

class NetTest;

namespace ooni {

Var<NetTest> DnsInjectionTest::create_test_() {
    OoniTestImpl *test = new DNSInjectionImpl(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    test->set_verbosity(verbosity);
    if (log_handler) test->on_log(log_handler);
    test->set_reactor(reactor);
    return Var<NetTest>(test);
}

Var<NetTest> HttpInvalidRequestLineTest::create_test_() {
    OoniTestImpl *test = new HTTPInvalidRequestLineImpl(settings);
    if (output_path != "") test->set_report_filename(output_path);
    test->set_verbosity(verbosity);
    if (log_handler) test->on_log(log_handler);
    test->set_reactor(reactor);
    return Var<NetTest>(test);
}

Var<NetTest> TcpConnectTest::create_test_() {
    OoniTestImpl *test = new TCPConnectImpl(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    test->set_verbosity(verbosity);
    if (log_handler) test->on_log(log_handler);
    test->set_reactor(reactor);
    return Var<NetTest>(test);
}

} // namespace ooni
} // namespace mk
