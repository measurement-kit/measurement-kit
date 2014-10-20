#include "report/file.hpp"

FileReporter::FileReporter(const std::string& test_name,
      const std::string& test_version, const time_t& start_time,
      const std::string& probe_ip, 
      const std::map<std::string, std::string>& options,
      const std::string& filename) : ReporterBase(test_name, test_version,
          start_time, probe_ip, options) {
  this->filename = filename;
}

void
FileReporter::open() {
  ReporterBase::open();
  file.open(filename);
  file << getHeader();
}

void
FileReporter::writeEntry(ReportEntry& entry) {
  ReporterBase::writeEntry(entry);
  file << entry.str();
}

void
FileReporter::close() {
  ReporterBase::close();
  file.close();
}
