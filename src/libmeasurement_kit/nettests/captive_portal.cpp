// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

CaptivePortalRunnable::CaptivePortalRunnable() noexcept {
    test_name = "captiveportal";
    test_version = "0.4.0";
}

void CaptivePortalRunnable::main(std::string input, Settings options,
                                 Callback<SharedPtr<report::Entry>> cb) {
    ooni::captiveportal(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
