// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"

#include <measurement_kit/report.hpp>

/*
 * Guess the platform in which we are.
 *
 * See: <https://sourceforge.net/p/predef/wiki/OperatingSystems/>
 *      <http://stackoverflow.com/a/18729350>
 */
#if defined __ANDROID__
#  define MK_PLATFORM "android"
#elif defined __linux__
#  define MK_PLATFORM "linux"
#elif defined _WIN32
#  define MK_PLATFORM "windows"
#elif defined __APPLE__
#  include <TargetConditionals.h>
#  if TARGET_OS_IPHONE
#    define MK_PLATFORM "ios"
#  else
#    define MK_PLATFORM "macos"
#  endif
#else
#  define MK_PLATFORM "unknown"
#endif

namespace mk {
namespace report {

Report::Report() {
    memset(&test_start_time, 0, sizeof (test_start_time));
}

void Report::add_reporter(Var<BaseReporter> reporter) {
    reporters_.push_back(reporter);
}

void Report::fill_entry(Entry &entry) const {
    entry["test_name"] = test_name;
    entry["test_version"] = test_version;
    entry["test_start_time"] = *mk::timestamp(&test_start_time);
    // header["options"] = options;
    entry["probe_ip"] = probe_ip;
    entry["probe_asn"] = probe_asn;
    entry["probe_cc"] = probe_cc;
    entry["software_name"] = options.get("software_name", software_name);
    entry["software_version"] =
        options.get("software_version", software_version);
    entry["data_format_version"] = data_format_version;
    entry["annotations"]["platform"] =
        options.get("platform", std::string{MK_PLATFORM});
    entry["annotations"]["engine_name"] = "libmeasurement_kit";
    entry["annotations"]["engine_version"] = MK_VERSION;
    entry["annotations"]["engine_version_full"] = MK_VERSION_FULL;
}

Entry Report::get_dummy_entry() const {
    Entry entry;
    fill_entry(entry);
    return entry;
}

#define FMAP mk::fmap<Var<BaseReporter>, Continuation<Error>>

void Report::open(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [=](Var<BaseReporter> r) {
        return r->open(*this);
    }), callback);
}

void Report::write_entry(Entry entry, Callback<Error> callback,
                         Var<Logger> logger) {
    if (report_id == "") {
        auto count = 0;
        for (auto &reporter : reporters_) {
            auto rid = reporter->get_report_id();
            if (rid != "") {
                report_id = rid;
                ++count;
            }
        }
        /*
         * XXX For now we trust the last reporter that provides us with a
         * non-zero-length report-id. The reason for doing the full scan
         * over the existing reporters and for checking is to be ready for
         * an hypothetical moment where more than one reporter will give
         * back a good report-id (only ooni_reporter does that).
         */
        if (count > 1) {
            callback(MultipleReportIdsError());
            return;
        }
        if (report_id != "") {
            logger->debug("report: found report-id: '%s'", report_id.c_str());
        }
    }

    /*
     * Override the report id with the one found above (if any). Note that
     * memoization to prevent submitting multiple times in case of a partial
     * failure and retry will be performed by write_entry() below.
     */
    if (report_id != "") {
        entry["report_id"] = report_id;
        logger->debug("report: force adding to entry report-id: %s",
            entry["report_id"].dump().c_str());
    }

    mk::parallel(FMAP(reporters_, [=](Var<BaseReporter> r) {
        return r->write_entry(entry);
    }), callback);
}

void Report::close(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [](Var<BaseReporter> r) {
        return r->close();
    }), callback);
}

#undef FMAP // So long, and thanks for the fish

} // namespace report
} // namespace mk
