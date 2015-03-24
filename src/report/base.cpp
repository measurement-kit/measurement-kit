#include <ight/report/base.hpp>

using namespace ight::report::base;

std::string
ReporterBase::getHeader() {
    std::stringstream output;
    YAML::Node header;
    header["test_name"] = test_name;
    header["test_version"] = test_version;
    header["start_time"] = start_time;
    // this->header["options"] = options;
    header["probe_ip"] = probe_ip;
    // this->header["probe_asn"] = probe_ip;
    // this->header["probe_cc"] = probe_ip;
    header["software_name"] = software_name;
    header["software_version"] = software_version;
    header["data_format_version"] = data_format_version;
    output << "---" << std::endl;
    output << header << std::endl;
    output << "..." << std::endl;
    return output.str();
}

void
ReporterBase::open() {
  openned = true;
}

void
ReporterBase::writeEntry(ReportEntry& entry) {
  if (!openned) {
    throw new std::runtime_error("The report is not open.");
  }
  if (closed) {
    throw new std::runtime_error("The report has already been closed.");
  }
  // This is here to silence compiler warnings
  (void) entry;
}

void
ReporterBase::close() {
  openned = false;
  closed = true;
}
