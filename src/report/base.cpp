#include "report/base.hpp"

ReporterBase::ReporterBase(const std::string& test_name, const std::string&
    test_version, const time_t& start_time, const std::string& probe_ip, const
    std::map<std::string, std::string>& options) : test_name(test_name),
  test_version(test_version), start_time(start_time), probe_ip(probe_ip),
  options(options) {
    // this->probe_asn;
    // this->probe_cc;
}

std::string
ReporterBase::getHeader() {
    std::stringstream output;
    YAML::Node header;
    header["test_name"] = test_name;
    header["test_version"] = test_version;
    // this->header["start_time"] = test_name;
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
}

void
ReporterBase::close() {
  openned = false;
  closed = true;
}
