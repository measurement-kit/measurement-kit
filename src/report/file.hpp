#ifndef LIBIGHT_FILE_REPORTER_HPP
# define LIBIGHT_FILE_REPORTER_HPP

#include <iostream>
#include <fstream>

#include "report/base.hpp"

class FileReporter : public ReporterBase {
  std::string filename;
  std::ofstream file;
public:
  FileReporter(const std::string& test_name, const std::string& test_version,
      const time_t& start_time, const std::string& probe_ip,
      const std::map<std::string, std::string>& options,
      const std::string& filename);

  FileReporter(const FileReporter& that) : test_name(that.test_name),
    test_version(that.test_version), probe_ip(that.probe_ip),
    start_time(that.start_time), options(that.options) {};

  void open();
  void writeEntry(ReportEntry& entry);
  void close();
};

#endif
