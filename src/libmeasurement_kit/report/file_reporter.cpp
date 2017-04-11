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

/* static */ Var<BaseReporter> FileReporter::make(std::string s) {
    Var<FileReporter> reporter(new FileReporter);
    reporter->filename = s;
    return reporter;
}

Continuation<Error> FileReporter::open(Report &) {
    return do_open_([=](Callback<Error> cb) {
        file.open(filename);
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    });
}

Continuation<Error> FileReporter::write_entry(Entry entry) {
    return do_write_entry_(entry, [=](Callback<Error> cb) {
        file << entry.dump() << std::endl;
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    });
}

Continuation<Error> FileReporter::close() {
    return do_close_([=](Callback<Error> cb) {
        file.close();
        if (!file.good()) {
            cb(map_error(file));
            return;
        }
        cb(NoError());
    });
}

} // namespace report
} // namespace mk
