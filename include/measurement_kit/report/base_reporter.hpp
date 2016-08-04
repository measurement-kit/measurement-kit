// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP
#define MEASUREMENT_KIT_REPORT_BASE_REPORTER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/report/entry.hpp>

namespace mk {
namespace report {

class BaseReporter {
  public:
    virtual ~BaseReporter() {}

    virtual Continuation<Error> open();

    virtual Continuation<Error> write_entry(const Entry &entry);

    virtual Continuation<Error> close();
};

} // namespace report
} // namespace mk
#endif
