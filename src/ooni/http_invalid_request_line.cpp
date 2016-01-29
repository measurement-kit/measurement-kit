// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni/http_invalid_request_line_test.hpp>

#include "src/ooni/http_invalid_request_line.hpp"
#include <sys/stat.h>

namespace mk {
namespace ooni {

Var<mk::NetTest> HttpInvalidRequestLineTest::create_test_() {
    OoniTest *test = new HTTPInvalidRequestLine(settings);
    if (output_path != "") test->set_report_filename(output_path);
    if (is_verbose) test->set_verbose(1);
    if (log_handler) test->on_log(log_handler);
    return Var<mk::NetTest>(test);
}

} // namespace ooni
} // namespace mk
