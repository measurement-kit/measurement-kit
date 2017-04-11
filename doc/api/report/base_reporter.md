# NAME
base_reporter &mdash; Base class for reporters

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

class BaseReporter : public NonCopyable, public NonMovable {
  public:
    virtual Continuation<Error> open(Report) {
        return do_open_([=](Callback<Error> cb) { cb(NoError()); });
    }

    virtual Continuation<Error> write_entry(Entry e) {
        return do_write_entry_(e, [=](Callback<Error> cb) { cb(NoError()); });
    }

    virtual Continuation<Error> close() {
        return do_close_([=](Callback<Error> cb) { cb(NoError()); });
    }

  protected:
    Continuation<Error> do_open_(Continuation<Error> cc);

    Continuation<Error> do_write_entry_(Entry, Continuation<Error> cc);

    Continuation<Error> do_close_(Continuation<Error> cc);
};

} // namespace report
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `BaseReporter` class is the base class for all reporters. By default its
`open`, `write_entry`, and `close` methods do nothing and return success. This
behavior is to be override by derived classes.

Derived classes MUST implement their `open`, `write_entry`, and `close`
overriden methods using the `do_open_`, `do_write_entry_` and `do_close_`
methods provided by the base class. Such three methods implement the
idempotent semantic that allows to retry `Report` operations succeeding
for some `BaseReports` and failing for others.

# HISTORY

The `BaseReporter` class appeared in MeasurementKit 0.3.0.
