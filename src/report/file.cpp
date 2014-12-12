#include "report/file.hpp"

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
