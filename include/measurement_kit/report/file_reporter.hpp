// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP

#include <iostream>
#include <fstream>

#include <measurement_kit/report/base_reporter.hpp>

namespace measurement_kit {
namespace report {

class FileReporter : public BaseReporter {
public:
  std::string filename;
  void open();
  void writeEntry(ReportEntry& entry);
  void close();
private:
  std::ofstream file;
};

} // namespace report
} // namespace measurement_kit
#endif
