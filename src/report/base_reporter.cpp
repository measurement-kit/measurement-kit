// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report/base_reporter.hpp>

namespace measurement_kit {
namespace report {

std::string BaseReporter::getHeader() {
    std::stringstream output;
    YAML::Node header;
    header["test_name"] = test_name;
    header["test_version"] = test_version;
    header["start_time"] = start_time;
    // header["options"] = options;
    header["probe_ip"] = probe_ip;
    // header["probe_asn"] = probe_ip;
    // header["probe_cc"] = probe_ip;
    header["software_name"] = software_name;
    header["software_version"] = software_version;
    header["data_format_version"] = data_format_version;
    output << "---" << std::endl;
    output << header << std::endl;
    output << "..." << std::endl;
    return output.str();
}

void BaseReporter::open() { openned = true; }

void BaseReporter::writeEntry(Entry &) {
    if (!openned) {
        throw new std::runtime_error("The report is not open.");
    }
    if (closed) {
        throw new std::runtime_error("The report has already been closed.");
    }
}

void BaseReporter::close() {
    openned = false;
    closed = true;
}

} // namespace report
} // namespace measurement_kit
