// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report/file.hpp>

namespace measurement_kit {
namespace report {

void FileReporter::open() {
  ReporterBase::open();
  file.open(filename);
  file << getHeader();
}

void FileReporter::writeEntry(ReportEntry& entry) {
  ReporterBase::writeEntry(entry);
  file << entry.str();
}

void FileReporter::close() {
  ReporterBase::close();
  file.close();
}

}}
