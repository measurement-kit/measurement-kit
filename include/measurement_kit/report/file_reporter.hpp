// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP

#include <fstream>
#include <measurement_kit/report/base_reporter.hpp>

namespace mk {
namespace report {

class FileReporter : public BaseReporter {
  public:
    std::string filename;

#define XX __attribute__((warn_unused_result))

    Error open() override XX;
    Error write_entry(Entry &entry) override XX;
    Error close() override XX;

#undef XX

  private:
    std::ofstream file;
};

} // namespace report
} // namespace mk
#endif
