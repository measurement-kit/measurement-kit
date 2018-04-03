// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/report/base_reporter.hpp"
#include "src/libmeasurement_kit/report/error.hpp"

namespace mk {
namespace report {

/* static */ SharedPtr<BaseReporter> BaseReporter::make() {
    return SharedPtr<BaseReporter>(new BaseReporter);
}

BaseReporter::~BaseReporter() {}

Continuation<Error> BaseReporter::do_open_(Continuation<Error> cc) {
    return [=](Callback<Error> cb) {
        if (openned_) {
            // Make the operation idempotent by not failing it, but still pass
            // a child error to tell the caller report was already openned
            cb(NoError(ReportAlreadyOpenError()));
            return;
        }
        cc([=](Error error) {
            if (error) {
                cb(error);
                return;
            }
            openned_ = true;
            cb(NoError());
        });
    };
}

Continuation<Error>
BaseReporter::do_write_entry_(Entry entry, Continuation<Error> cc) {
    return [=](Callback<Error> cb) {
        if (!openned_) {
            cb(ReportNotOpenError());
            return;
        }
        if (closed_) {
            cb(ReportAlreadyClosedError());
            return;
        }
        // On success we save the serialization of the previous entry such
        // that submitting more than once the same entry is idempontent
        std::string serio = entry.dump();
        if (serio == prev_entry_) {
            cb(NoError(DuplicateEntrySubmitError()));
            return;
        }
        cc([=](Error error) {
            if (error) {
                cb(error);
                return;
            }
            prev_entry_ = serio; // Only on success to allow resubmit
            cb(NoError());
        });
    };
}

Continuation<Error> BaseReporter::do_close_(Continuation<Error> cc) {
    return [=](Callback<Error> cb) {
        if (closed_) {
            // Make the operation idempotent by not failing it, but still pass
            // a child error to tell the caller report was already openned
            cb(NoError(ReportAlreadyClosedError()));
            return;
        }
        cc([=](Error error) {
            if (error) {
                cb(error);
                return;
            }
            closed_ = true;
            cb(NoError());
        });
    };
}

} // namespace report
} // namespace mk
