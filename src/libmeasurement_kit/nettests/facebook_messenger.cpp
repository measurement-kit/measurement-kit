// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

FacebookMessengerTest::FacebookMessengerTest() : BaseTest() {
    runnable.reset(new FacebookMessengerRunnable);
    runnable->test_name = "facebook_messenger";
    runnable->test_version = "0.0.2";
    runnable->needs_input = false;
}

void FacebookMessengerRunnable::main(std::string /*input*/, Settings options,
                            Callback<SharedPtr<report::Entry>> cb) {
    ooni::facebook_messenger(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
