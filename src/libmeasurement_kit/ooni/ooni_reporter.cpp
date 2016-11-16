// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

OoniReporter::OoniReporter(const OoniTest &parent) {
    settings = parent.options; // Copy the parent's settings
    logger = parent.logger;
    reactor = parent.reactor;
    if (settings.find("collector_base_url") == settings.end()) {
        // Note: by default we use the testing collector URL because otherwise
        // testing runs would be collected creating noise and using resources
        settings["collector_base_url"] = collector::testing_collector_url();
    }
    logger->info("Using collector: %s", settings["collector_base_url"].c_str());
}

/* static */ Var<BaseReporter> OoniReporter::make(const OoniTest &parent) {
    Var<OoniReporter> reporter(new OoniReporter(parent));
    return reporter;
}

Continuation<Error> OoniReporter::open(Report report) {
    return do_open_([=](Callback<Error> cb) {
        logger->info("Opening report...");
        collector::connect_and_create_report(
                report.get_dummy_entry(),
                [=](Error error, std::string report_id) {
                    logger->info("Opening report... %d", error.code);
                    if (not error) {
                        logger->info("Report ID: %s", report_id.c_str());
                        this->report_id = report_id;
                    }
                    cb(error);
                },
                settings,
                reactor,
                logger);
    });
}

Continuation<Error> OoniReporter::write_entry(Entry entry) {
    /*
     * Unconditionally overwrite the `report_id` field with what was
     * passed us by the server, which should be authoritative.
     *
     * This action must be performed here rather than below because in
     * the lambda context `entry` would be read only.
     *
     * Note that `entry` is passed by copy so changing it has no
     * effect outside of this function.
     */
    entry["report_id"] = report_id;

    // Register action for when we will be asked to write the entry
    return do_write_entry_(entry, [=](Callback<Error> cb) {
        if (report_id == "") {
            logger->warn("ooni_reporter: missing report ID");
            cb(MissingReportIdError());
            return;
        }
        logger->info("Submitting entry...");
        collector::connect_and_update_report(report_id, entry,
                                             [=](Error e) {
                                                 logger->info(
                                                     "Submitting entry... %d",
                                                     e.code);
                                                 cb(e);
                                             },
                                             settings,
                                             reactor,
                                             logger);
    });
}

Continuation<Error> OoniReporter::close() {
    return do_close_([=](Callback<Error> cb) {
        if (report_id == "") {
            logger->warn("ooni_reporter: missing report ID");
            cb(MissingReportIdError());
            return;
        }
        logger->info("Closing report...");
        collector::connect_and_close_report(report_id,
                                            [=](Error e) {
                                                logger->info(
                                                    "Closing report... %d",
                                                    e.code);
                                                cb(e);
                                            },
                                            settings,
                                            reactor,
                                            logger);
    });
}

} // namespace ooni
} // namespace mk
