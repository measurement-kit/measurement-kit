// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

FacebookMessengerTest::FacebookMessengerTest() : BaseTest() {
    runnable.reset(new FacebookMessengerRunnable);
    runnable->test_name = "facebook_messenger";
    runnable->test_version = "0.0.1";
    runnable->needs_input = false;
}

void FacebookMessengerRunnable::main(std::string input, Settings options,
                            Callback<Var<report::Entry>> cb) {
    ooni::facebook_messenger(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
