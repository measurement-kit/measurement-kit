// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/report/base_reporter.hpp"

namespace mk {
namespace report {

void BaseReporter::open() { openned_ = true; }

void BaseReporter::write_entry(json &entry) {
    if (!openned_) {
        throw new std::runtime_error("The report is not open.");
    }
    if (closed_) {
        throw new std::runtime_error("The report has already been closed.");
    }
    entry["test_name"] = test_name;
    entry["test_version"] = test_version;
    entry["test_start_time"] = mk::timestamp(&test_start_time);
    // header["options"] = options;
    entry["probe_ip"] = probe_ip;
    entry["probe_asn"] = probe_asn;
    entry["probe_cc"] = probe_cc;
    entry["software_name"] = software_name;
    entry["software_version"] = software_version;
    entry["data_format_version"] = data_format_version;
}

void BaseReporter::close() {
    openned_ = false;
    closed_ = true;
}

} // namespace report
} // namespace mk
