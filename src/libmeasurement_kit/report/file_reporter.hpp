// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP
#define SRC_LIBMEASUREMENT_KIT_REPORT_FILE_REPORTER_HPP

#include <fstream>

#include "src/libmeasurement_kit/report/base_reporter.hpp"

namespace mk {
namespace report {

class FileReporter : public BaseReporter {
  public:
    static SharedPtr<BaseReporter> make(std::string filename);

    Continuation<Error> open(Report &report) override;
    Continuation<Error> write_entry(Entry entry) override;
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
