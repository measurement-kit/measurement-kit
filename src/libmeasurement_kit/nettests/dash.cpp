// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/runnable.hpp"
#include <measurement_kit/nettests.hpp>
#include <measurement_kit/neubot.hpp>

namespace mk {
namespace nettests {

DashTest::DashTest() : BaseTest() {
    runnable.reset(new DashRunnable);
    runnable->test_name = "dash";
    runnable->test_version = "0.7.0";
    runnable->needs_input = false;
}

void DashRunnable::main(std::string /*input*/, Settings options,
                        Callback<Var<report::Entry>> cb) {
    auto entry = Var<report::Entry>::make();
    neubot::dash::negotiate(entry, options, reactor, logger, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.as_ooni_error();
        } else {
            (*entry)["failure"] = nullptr;
        }
        cb(entry);
    });
}

} // namespace nettests
} // namespace mk
