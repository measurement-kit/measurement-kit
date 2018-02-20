// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

HttpHeaderFieldManipulationTest::HttpHeaderFieldManipulationTest() : BaseTest() {
    runnable.reset(new HttpHeaderFieldManipulationRunnable);
    runnable->test_name = "http_header_field_manipulation";
    runnable->test_version = "0.0.1";
    runnable->needs_input = false;
    runnable->test_helpers_data = {{"http-return-json-headers", "backend"}};
}

void HttpHeaderFieldManipulationRunnable::main(std::string input,
                                               Settings options,
                                               Callback<SharedPtr<report::Entry>> cb) {
    ooni::http_header_field_manipulation(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
