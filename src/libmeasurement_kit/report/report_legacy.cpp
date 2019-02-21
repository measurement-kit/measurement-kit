// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/fmap.hpp"
#include "src/libmeasurement_kit/common/parallel.hpp"

#include "src/libmeasurement_kit/common/utils.hpp"

#include "src/libmeasurement_kit/report/base_reporter.hpp"
#include "src/libmeasurement_kit/report/error.hpp"
#include "src/libmeasurement_kit/report/report_legacy.hpp"

namespace mk {
namespace report {

ReportLegacy::ReportLegacy() {}

void ReportLegacy::add_reporter(SharedPtr<BaseReporter> reporter) {
    reporters_.push_back(reporter);
}

void ReportLegacy::fill_entry(nlohmann::json &entry) const {
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
        options.get("platform", std::string{mk_platform()});
    entry["annotations"]["engine_name"] = "libmeasurement_kit";
    entry["annotations"]["engine_version"] = MK_VERSION;
    entry["annotations"]["engine_version_full"] = MK_VERSION_FULL;
    entry["report_id"] = report_id;
}

nlohmann::json ReportLegacy::get_dummy_entry() const {
    nlohmann::json entry;
    fill_entry(entry);
    return entry;
}

#define FMAP mk::fmap<SharedPtr<BaseReporter>, Continuation<Error>>

void ReportLegacy::open(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [=](SharedPtr<BaseReporter> r) {
        return r->open(*this);
    }), callback);
}

void ReportLegacy::write_entry(nlohmann::json entry, Callback<Error> callback,
                         SharedPtr<Logger>) {
    mk::parallel(FMAP(reporters_, [=](SharedPtr<BaseReporter> r) {
        return r->write_entry(entry);
    }), callback);
}

void ReportLegacy::close(Callback<Error> callback) {
    mk::parallel(FMAP(reporters_, [](SharedPtr<BaseReporter> r) {
        return r->close();
    }), callback);
}

#undef FMAP // So long, and thanks for the fish

} // namespace report
} // namespace mk
