// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"

#include "src/libmeasurement_kit/ndt/utils.hpp"
#include "src/libmeasurement_kit/ndt/run.hpp"

namespace mk {
namespace nettests {

NdtRunnable::NdtRunnable() noexcept {
    test_name = "ndt";
    test_version = "0.1.0";
}

void NdtRunnable::main(std::string, Settings settings,
                       Callback<SharedPtr<nlohmann::json>> cb) {
    SharedPtr<nlohmann::json> entry(new nlohmann::json);
    (*entry)["failure"] = nullptr;
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object
    ndt::run(entry, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.reason;
        }
        try {
            (*entry)["simple"] = mk::ndt::utils::compute_simple_stats_throws(
                *entry, logger);
        } catch (const std::exception &) {
            (*entry)["failure"] = "compute_simple_stats_error";
        }
        try {
            (*entry)["advanced"] =
                mk::ndt::utils::compute_advanced_stats_throws(*entry, logger);
        } catch (const std::exception &) {
            (*entry)["failure"] = "compute_advanced_stats_error";
        }
        cb(entry);
    }, settings, reactor, logger);
}

} // namespace nettests
} // namespace mk
