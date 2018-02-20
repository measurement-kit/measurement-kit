// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>

#include "src/libmeasurement_kit/ndt/utils.hpp"
#include "src/libmeasurement_kit/ndt/run.hpp"

namespace mk {
namespace nettests {

MultiNdtTest::MultiNdtTest() : BaseTest() {
    runnable.reset(new MultiNdtRunnable);
    runnable->options["save_real_probe_ip"] = true;
    runnable->options["dns/engine"] = "system";
    runnable->test_name = "multi_ndt";
    runnable->test_version = "0.1.0";  /* Forked from `ndt` v0.0.4 */
}

static void write_simple_stats(report::Entry &entry, SharedPtr<Logger> logger) {
    report::Entry single = mk::ndt::utils::compute_simple_stats(entry["single_stream"], logger);
    single["fastest_test"] = "single_stream";
    report::Entry multi = mk::ndt::utils::compute_simple_stats(entry["multi_stream"], logger);
    multi["fastest_test"] = "multi_stream";

    report::Entry selected;

    /*
     * Here we basically pick up the fastest of the two tests.
     */
    if (single["ping"] != nullptr and multi["ping"] != nullptr) {
        if (single["download"] != nullptr and multi["download"] != nullptr) {
            double singled = single["download"];
            double multid = multi["download"];
            if (singled > multid) {
                selected = single;
            } else {
                selected = multi;
            }
        } else if (single["download"] != nullptr) {
            logger->warn("Multi-stream download is null");
            selected = single;
        } else if (multi["download"] != nullptr) {
            logger->warn("Single-stream download is null");
            selected = multi;
        } else {
            logger->warn("Single- and multi-stream download are null");
        }
    } else if (single["ping"] != nullptr) {
        logger->warn("Multi-stream ping is null");
        selected = single;
    } else if (multi["ping"] != nullptr) {
        logger->warn("Single-stream ping is null");
        selected = multi;
    } else {
        logger->warn("Single- and multi-stream ping are null");
    }

    entry["simple"] = selected;
}

void MultiNdtRunnable::main(std::string, Settings ndt_settings,
                            Callback<SharedPtr<report::Entry>> cb) {
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object

    SharedPtr<report::Entry> ndt_entry(new report::Entry);
    (*ndt_entry)["failure"] = nullptr;
    // By default we only run download but let's allow clients to decide
    if (ndt_settings.count("single_test_suite") != 0) {
        ndt_settings["test_suite"] = ndt_settings["single_test_suite"];
    } else {
        ndt_settings["test_suite"] = MK_NDT_DOWNLOAD;
    }
    logger->set_progress_offset(0.15);
    logger->set_progress_scale(0.35);
    logger->progress(0.0, "Starting single-stream test");
    ndt::run(ndt_entry, [=](Error ndt_error) {
        if (ndt_error) {
            (*ndt_entry)["failure"] = ndt_error.reason;
            logger->warn("Test failed: %s", ndt_error.what());
            // FALLTHROUGH
        }

        SharedPtr<report::Entry> neubot_entry(new report::Entry);
        (*neubot_entry)["failure"] = nullptr;
        Settings neubot_settings{ndt_settings.begin(), ndt_settings.end()};
        neubot_settings["test_suite"] = MK_NDT_DOWNLOAD_EXT;
        neubot_settings["mlabns_tool_name"] = "neubot";
        logger->set_progress_offset(0.55);
        logger->set_progress_scale(0.35);
        logger->progress(0.0, "Starting multi-stream test");
        ndt::run(neubot_entry, [=](Error neubot_error) mutable {
            logger->progress(1.0, "Test completed");
            if (neubot_error) {
                (*neubot_entry)["failure"] = neubot_error.reason;
                logger->warn("Test failed: %s", neubot_error.what());
                // FALLTHROUGH
            }
            SharedPtr<report::Entry> overall_entry(new report::Entry);
            (*overall_entry)["failure"] = nullptr;
            (*overall_entry)["multi_stream"] = *neubot_entry;
            (*overall_entry)["single_stream"] = *ndt_entry;
            if (ndt_error or neubot_error) {
                Error overall_error = SequentialOperationError();
                overall_error.add_child_error(std::move(ndt_error));
                overall_error.add_child_error(std::move(neubot_error));
                (*overall_entry)["failure"] = overall_error.reason;
                // FALLTHROUGH
            }
            try {
                write_simple_stats(*overall_entry, logger);
            } catch (const std::exception &) {
                /* Just in case */ ;
            }
            try {
                (*overall_entry)["advanced"] =
                    mk::ndt::utils::compute_advanced_stats(
                        (*overall_entry)["single_stream"], logger);
            } catch (const std::exception &) {
                /* Just in case */ ;
            }
            cb(overall_entry);
        }, neubot_settings, reactor, logger);
    }, ndt_settings, reactor, logger);
}

} // namespace nettests
} // namespace mk
