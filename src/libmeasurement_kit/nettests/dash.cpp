// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/neubot/dash.hpp"

namespace mk {
namespace nettests {

DashRunnable::DashRunnable() noexcept {
    test_name = "dash";
    test_version = "0.7.0";
    needs_input = false;
}

void DashRunnable::main(std::string /*input*/, Settings options,
                        Callback<SharedPtr<report::Entry>> cb) {
    auto entry = SharedPtr<report::Entry>::make();
    neubot::dash::negotiate(entry, options, reactor, logger, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.reason;
        } else {
            (*entry)["failure"] = nullptr;
        }
        cb(entry);
    });
}

} // namespace nettests
} // namespace mk
