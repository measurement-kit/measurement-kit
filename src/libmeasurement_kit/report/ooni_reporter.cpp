// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <measurement_kit/report.hpp>

#include "src/libmeasurement_kit/ooni/collector_client.hpp"

namespace mk {
namespace report {

OoniReporter::OoniReporter(Settings s, SharedPtr<Reactor> r, SharedPtr<Logger> l) {
    settings = s;
    reactor = r;
    logger = l;
    if (settings.find("collector_base_url") == settings.end()) {
        // Note: by default we use the production collector URL and we need
        // to remember to switch to the testing one in tests.
        settings["collector_base_url"] =
            ooni::collector::production_collector_url();
    }
    logger->info("Results collector: %s",
        settings["collector_base_url"].c_str());
}

/* static */ SharedPtr<BaseReporter> OoniReporter::make(Settings settings,
        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    SharedPtr<OoniReporter> reporter(new OoniReporter(settings, reactor, logger));
    return reporter.as<BaseReporter>();
}

Continuation<Error> OoniReporter::open(Report &report) {
    return do_open_([=](Callback<Error> cb) {
        logger->info("Opening report; please be patient...");
        ooni::collector::connect_and_create_report(
                report.get_dummy_entry(),
                [=](Error error, std::string rid) {
                    logger->debug("Opening report... %d", error.code);
                    if (not error) {
                        logger->info("Report ID: %s", rid.c_str());
                        report_id = rid;
                    }
                    cb(error);
                },
                settings,
                reactor,
                logger);
    });
}

Continuation<Error> OoniReporter::write_entry(Entry entry) {

    // Register action for when we will be asked to write the entry
    return do_write_entry_(entry, [=](Callback<Error> cb) {
        if (report_id == "") {
            logger->warn("ooni_reporter: missing report ID");
            cb(MissingReportIdError());
            return;
        }
        logger->info("Submitting test results; please be patient...");
        ooni::collector::connect_and_update_report(report_id, entry,
                                             [=](Error e) {
                                                 logger->debug(
                                                     "Submitting entry... %d",
                                                     e.code);
                                                 if (!e) {
                                                     logger->info("Results "
                                                        "successfully "
                                                        "submitted");
                                                 }
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
        logger->info("Closing report; please be patient...");
        ooni::collector::connect_and_close_report(report_id,
                                            [=](Error e) {
                                                logger->debug(
                                                    "Closing report... %d",
                                                    e.code);
                                                if (!e) {
                                                    logger->info("Report "
                                                        "successfully closed");
                                                }
                                                cb(e);
                                            },
                                            settings,
                                            reactor,
                                            logger);
    });
}

std::string OoniReporter::get_report_id() {
    return report_id;
}

} // namespace report
} // namespace mk
