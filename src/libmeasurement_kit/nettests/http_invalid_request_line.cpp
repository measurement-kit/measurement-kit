// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

HttpInvalidRequestLineTest::HttpInvalidRequestLineTest() : BaseTest() {
    runnable.reset(new HttpInvalidRequestLineRunnable);
    runnable->test_name = "http_invalid_request_line";
    runnable->test_version = "0.0.3";
    runnable->test_helpers_data = {{"tcp-echo", "backend"}};
}

void HttpInvalidRequestLineRunnable::main(std::string, Settings options,
                                          Callback<SharedPtr<report::Entry>> cb) {
    ooni::http_invalid_request_line(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
