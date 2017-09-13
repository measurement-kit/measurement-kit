// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ndt.hpp>

#include "private/ndt/utils.hpp"

namespace mk {
namespace nettests {

NdtTest::NdtTest() : BaseTest() {
    runnable.reset(new NdtRunnable);
    runnable->test_name = "ndt";
    runnable->test_version = "0.1.0";
}

void NdtRunnable::main(std::string, Settings settings,
                       Callback<SharedPtr<report::Entry>> cb) {
    SharedPtr<report::Entry> entry(new report::Entry);
    (*entry)["failure"] = nullptr;
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object
    ndt::run(entry, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.as_ooni_error();
        }
        try {
            (*entry)["simple"] = mk::ndt::utils::compute_simple_stats(*entry, logger);
        } catch (const std::exception &) {
            /* Just in case */ ;
        }
        try {
            (*entry)["advanced"] = mk::ndt::utils::compute_advanced_stats(*entry, logger);
        } catch (const std::exception &) {
            /* Just in case */ ;
        }
        cb(entry);
    }, settings, reactor, logger);
}

} // namespace nettests
} // namespace mk
