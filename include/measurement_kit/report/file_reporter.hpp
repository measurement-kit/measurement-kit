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
    static Var<FileReporter> make(std::string filename);

    Continuation<Error> open() override;
    Continuation<Error> write_entry(const Entry &entry) override;
    Continuation<Error> close() override;

    ~FileReporter() override {}

  private:
    FileReporter() {}

    std::string filename;
    std::ofstream file;
};

} // namespace report
} // namespace mk
#endif
