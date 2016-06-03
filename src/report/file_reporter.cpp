// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report.hpp>

using json = nlohmann::json;

namespace mk {
namespace report {

void FileReporter::open() {
    BaseReporter::open();
    try {
        file.open(filename);
    } catch (...) {
        emit_error(GenericError());
    }
}

void FileReporter::write_entry(report::Entry &entry) {
    BaseReporter::write_entry(entry);
    try {
        file << entry.dump() << std::endl;
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
