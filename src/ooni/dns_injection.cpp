// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni/dns_injection_test.hpp>

#include "src/ooni/dns_injection.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

Var<mk::NetTest> DnsInjectionTest::create_test_() {
    OoniTest *test = new DNSInjection(input_path, settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    return Var<mk::NetTest>(test);
}

} // namespace ooni
} // namespace mk
