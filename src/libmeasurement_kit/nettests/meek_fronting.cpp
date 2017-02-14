// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

MeekFrontingTest::MeekFrontingTest() : BaseTest() {
    runnable.reset(new MeekFrontingRunnable);
    runnable->test_name = "meek_fronting";
    runnable->test_version = "0.0.1";
    runnable->needs_input = true;
}

void MeekFrontingRunnable::main(std::string input, Settings options,
                                Callback<Var<report::Entry>> cb) {
    ooni::meek_fronting(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
