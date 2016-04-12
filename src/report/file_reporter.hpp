// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP

#include <iostream>
#include <fstream>

#include "src/report/base_reporter.hpp"

namespace mk {
namespace report {

class FileReporter : public BaseReporter {
  public:
    std::string filename;
    void open() override;
    void writeEntry(json &entry) override;
    void close() override;

  private:
    std::ofstream file;
};

} // namespace report
} // namespace mk
#endif
