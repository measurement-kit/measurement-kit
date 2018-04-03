// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP
#define SRC_LIBMEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP

#include "src/libmeasurement_kit/report/report.hpp"
#include "src/libmeasurement_kit/report/entry.hpp"

namespace mk {
namespace report {

class Report;

class BaseReporter : public NonCopyable, public NonMovable {
  public:
    static SharedPtr<BaseReporter> make();

    virtual ~BaseReporter();

    // TODO: refactor moving these inline functions inside of the .cpp file

    virtual Continuation<Error> open(Report &) {
        return do_open_([=](Callback<Error> cb) { cb(NoError()); });
    }

    virtual Continuation<Error> write_entry(Entry e) {
        return do_write_entry_(e, [=](Callback<Error> cb) { cb(NoError()); });
    }

    virtual Continuation<Error> close() {
        return do_close_([=](Callback<Error> cb) { cb(NoError()); });
    }

    virtual std::string get_report_id() {
        return ""; /* This is basically "invalid report id" */
    }

  protected:
    BaseReporter() {}

    Continuation<Error> do_open_(Continuation<Error> cc);

    Continuation<Error> do_write_entry_(Entry, Continuation<Error> cc);

    Continuation<Error> do_close_(Continuation<Error> cc);

  private:

    bool openned_ = false;
    bool closed_ = false;
    std::string prev_entry_;
};

} // namespace report
} // namespace mk
#endif
