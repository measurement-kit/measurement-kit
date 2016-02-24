// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>                              // for seconds
#include <functional>                          // for function
#include <measurement_kit/common/async.hpp>    // for Async
#include <measurement_kit/ooni/base_test.hpp>  // for BaseTest
#include <measurement_kit/ooni/dns_injection_test.hpp>
#include <measurement_kit/ooni/http_invalid_request_line_test.hpp>
#include <measurement_kit/ooni/tcp_connect_test.hpp>
#include <measurement_kit/common/var.hpp>      // for Var
#include <ratio>                               // for ratio
#include <sys/stat.h>
#include <thread>                              // for sleep_for
#include "src/ooni/dns_injection_impl.hpp"
#include "src/ooni/http_invalid_request_line_impl.hpp"
#include "src/ooni/tcp_connect_impl.hpp"

namespace mk {

class NetTest;

namespace ooni {

void BaseTest::run() {
    // XXX Ideally it would be best to run this in the current thread with
    // a dedicated poller, but the code is not yet ready for that.
    bool done = false;
    run([&done]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

void BaseTest::run(std::function<void()> callback) {
    Async::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

Var<NetTest> DnsInjectionTest::create_test_() {
    OoniTestImpl *test = new DNSInjectionImpl(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    test->set_poller(poller);
    return Var<NetTest>(test);
}

Var<NetTest> HttpInvalidRequestLineTest::create_test_() {
    OoniTestImpl *test = new HTTPInvalidRequestLineImpl(settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    test->set_poller(poller);
    return Var<NetTest>(test);
}

Var<NetTest> TcpConnectTest::create_test_() {
    OoniTestImpl *test = new TCPConnectImpl(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    test->set_poller(poller);
    return Var<NetTest>(test);
}

} // namespace ooni
} // namespace mk
