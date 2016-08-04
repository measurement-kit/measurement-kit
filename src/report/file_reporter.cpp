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

/* static */ Var<FileReporter> FileReporter::make(std::string s) {
    Var<FileReporter> file_reporter(new FileReporter);
    file_reporter->filename = s;
    return file_reporter;
}

Continuation<Error> FileReporter::open() {
    return [=](Callback<Error> cb) {
        file.open(filename);
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    };
}

Continuation<Error> FileReporter::write_entry(const Entry &entry) {
    std::string s = entry.dump();
    return [=](Callback<Error> cb) {
        file << s << std::endl;
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    };
}

Continuation<Error> FileReporter::close() {
    return [=](Callback<Error> cb) {
        file.close();
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    };
}

} // namespace report
} // namespace mk
