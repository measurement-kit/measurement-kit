// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace nettests {

NdtTest::NdtTest() : BaseTest() {
    runnable.reset(new NdtRunnable);
    runnable->options["save_real_probe_ip"] = true;
    runnable->test_name = "ndt";
    runnable->test_version = "0.0.4";
}

void NdtRunnable::main(std::string, Settings settings,
                       Callback<Var<report::Entry>> cb) {
    Var<report::Entry> entry(new report::Entry);
    (*entry)["failure"] = nullptr;
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object
    ndt::run(entry, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.as_ooni_error();
        }
        cb(entry);
    }, settings, reactor, logger);
}

} // namespace nettests
} // namespace mk
