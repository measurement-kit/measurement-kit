// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

static Error map_error(std::ofstream &file) {
    if (file.eof()) {
        return ReportEofError();
    }
    if (file.bad()) {
        return ReportIoError();
    }
    if (file.fail()) {
        return ReportLogicalError();
    }
    return GenericError();
}

Error FileReporter::open() {
    Error error = BaseReporter::open();
    if (error) {
        return error;
    }
    file.open(filename);
    if (!file.good()) {
        return map_error(file);
    }
    return NoError();
}

Error FileReporter::write_entry(report::Entry &entry) {
    Error error = BaseReporter::write_entry(entry);
    if (error) {
        return error;
    }
    file << entry.dump() << std::endl;
    if (!file.good()) {
        return map_error(file);
    }
    return NoError();
}

Error FileReporter::close() {
    Error error = BaseReporter::close();
    if (error) {
        return error;
    }
    file.close();
    if (!file.good()) {
        return map_error(file);
    }
    return NoError();
}

} // namespace report
} // namespace mk
