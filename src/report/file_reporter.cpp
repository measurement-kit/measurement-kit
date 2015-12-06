// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/report/file_reporter.hpp"

namespace mk {
namespace report {

void FileReporter::open() {
    BaseReporter::open();
    try {
        file.open(filename);
        file << getHeader();
    } catch (...) {
        emit_error(GenericError());
    }
}

void FileReporter::writeEntry(Entry &entry) {
    BaseReporter::writeEntry(entry);
    try {
        file << entry.str();
    } catch (...) {
        emit_error(GenericError());
    }
}

void FileReporter::close() {
    BaseReporter::close();
    try {
        file.close();
    } catch (...) {
        emit_error(GenericError());
    }
}

} // namespace report
} // namespace mk
