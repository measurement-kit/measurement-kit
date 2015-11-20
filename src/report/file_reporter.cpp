// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report/file_reporter.hpp>

namespace measurement_kit {
namespace report {

void FileReporter::open() {
    BaseReporter::open();
    file.open(filename);
    file << getHeader();
}

void FileReporter::writeEntry(ReportEntry &entry) {
    BaseReporter::writeEntry(entry);
    file << entry.str();
}

void FileReporter::close() {
    BaseReporter::close();
    file.close();
}

} // namespace report
} // namespace measurement_kit
