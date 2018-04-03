// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/report/error.hpp"
#include "src/libmeasurement_kit/report/file_reporter.hpp"

namespace mk {
namespace report {

static Error map_error(std::ostream &file) {
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

/* static */ SharedPtr<BaseReporter> FileReporter::make(std::string s) {
    SharedPtr<FileReporter> reporter(new FileReporter);
    reporter->filename = s;
    return reporter.as<BaseReporter>();
}

Continuation<Error> FileReporter::open(Report &) {
    return do_open_([=](Callback<Error> cb) {
        if (filename == "-") {
            cb(NoError());
            return;
        }
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
        std::ostream &frf = (filename == "-") ? std::cout : file;
        frf << entry.dump() << std::endl;
        if (!frf.good()) {
            cb(map_error(frf));
            return;
        }
        cb(NoError());
    });
}

Continuation<Error> FileReporter::close() {
    return do_close_([=](Callback<Error> cb) {
        if (filename == "-") {
            cb(NoError());
            return;
        }
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
