// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <private/nettests/runnable.hpp>

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

DpiFragmentTest::DpiFragmentTest() : BaseTest() {
    runnable.reset(new DpiFragmentRunnable);
    runnable->test_name = "dpi_fragment";
    runnable->test_version = "0.0.2";
    runnable->needs_input = false;
}

void DpiFragmentRunnable::main(std::string /*input*/, Settings options,
                            Callback<SharedPtr<report::Entry>> cb) {
    ooni::dpi_fragment(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
