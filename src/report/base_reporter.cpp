// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <cstring>
#include <measurement_kit/report.hpp>
#include "src/common/utils.hpp"

namespace mk {
namespace report {

BaseReporter::BaseReporter() {
    memset(&test_start_time, 0, sizeof (test_start_time));
}

Error BaseReporter::open() {
    if (openned_) {
        return ReportAlreadyOpen();
    }
    openned_ = true;
    return NoError();
}

Error BaseReporter::write_entry(report::Entry &entry) {
    if (!openned_) {
        return ReportNotOpen();
    }
    if (closed_) {
        return ReportAlreadyClosed();
    }
    entry["test_name"] = test_name;
    entry["test_version"] = test_version;
    entry["test_start_time"] = *mk::timestamp(&test_start_time);
    // header["options"] = options;
    entry["probe_ip"] = probe_ip;
    entry["probe_asn"] = probe_asn;
    entry["probe_cc"] = probe_cc;
    entry["software_name"] = software_name;
    entry["software_version"] = software_version;
    entry["data_format_version"] = data_format_version;
    return NoError();
}

Error BaseReporter::close() {
    if (closed_) {
        return ReportAlreadyClosed();
    }
    closed_ = true;
    return NoError();
}

} // namespace report
} // namespace mk
