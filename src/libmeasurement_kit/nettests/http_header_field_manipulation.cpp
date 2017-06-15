// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

HttpHeaderFieldManipulationTest::HttpHeaderFieldManipulationTest() : BaseTest() {
    runnable.reset(new HttpHeaderFieldManipulationRunnable);
    runnable->test_name = "http_header_field_manipulation";
    runnable->test_version = "0.0.1";
    runnable->needs_input = false;
    runnable->test_helpers_names = {"backend"};
}

void HttpHeaderFieldManipulationRunnable::main(std::string input,
                                               Settings options,
                                               Callback<Var<report::Entry>> cb) {
    ooni::http_header_field_manipulation(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
