# NAME
report &mdash; Structure representing a report

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

class Report {
  public:
    const std::string software_name = "measurement_kit";
    const std::string software_version = MEASUREMENT_KIT_VERSION;
    const std::string data_format_version = "0.2.0";

    std::string test_name;
    std::string test_version;
    std::string probe_ip;
    std::string probe_asn;
    std::string probe_cc;
    tm test_start_time;
    Settings options;

    Report();
    void add_reporter(SharedPtr<BaseReporter> reporter);
    void fill_entry(Entry &entry) const;
    Entry get_dummy_entry() const;
    void open(Callback<Error> callback);
    void write_entry(Entry entry, Callback<Error> callback);
    void close(Callback<Error> callback);
};

} // namespace report
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `Report` class contains the results of a test. In turn, a test is
composed of one or more measurements. Each measurement produces a report
entry. It is the test responsibility to instantiate the `Report`, to
properly fill all the public attributes of the `Report`, to `open()` the
report at the beginning of the test, to `write_entry()` whenever a new
entry is produced, to `close()` the report at the end of the test.

A `Report` can be associated to one or more reporter objects inheriting
from the `BaseReporter` class. Such reporters define the behavior of a
`Report` when it is openned, when a entry is submitted, when the report
is closed. The default `BaseReporter` class does nothing, but there
are more specialized classes, e.g. `FileReporter` that writes the report
onto a specified file, and `ooni::OoniReporter` that submits the report
to the OONI collector service.

The `add_reporter` method allows to associate an implementation of the
`BaseReporter` interface to the current report. This method can be called
more than once; each time it is called it appends the specified reporter
object to the list of reporters to be used by this class. Note that you
SHOULD NOT add reporters once you have `open()`ed the report.

The `fill_entry` method takes in input a `Entry` and fills its generic
fields by copying the public `Report` fields that the test owning the report
should have initialized.

The `get_dummy_entry` method instantiates an empty `Entry`, initializes it
by calling `fill_entry`, and then returns it. This is used by OONI's reporter
to pass an empty entry representing the whole report to OONI's collector.

The `open` method opens the report by calling the `open` method of each
registered reporter. This method MUST be idempotent, i.e. if a reporter's
`open` succeeds and you call again the report's `open`, the `open` of
the reporter would not be called again. This allows you to safely call
again `open` if *some* of the reporters did not open correctly, without
affecting the state of the reporters that did open correctly.

The `write_entry` method writes the entry by calling the corresponding
method of each registered reporter. Also this method is idempotent, i.e.
also in this case you can call `write_entry` again if some reporters
failed without writing more than once the same entry for the reporters
that succeeded.

The `close` method closes the report by calling the corresponding method
of each registered repoter. Also this method is idempotent, i.e. if not
all reporters correctly closed, you can call it again and the `close` would
be retried only for the reports that failed to close.

The `open`, `write_entry`, and `close` methods are asynchronous and you MUST
wait for them to complete and call the `callback` provided as argument. In
case of success, `NoError()` is passed to the callback; otherwise, the error
passed to the callback indicates what went wrong. Specifically, since each
of these three operations runs in parallel the indicated operation to all the
registered reporters, in case of failure the error should be of type
`ParallelOperationError`. To investigate more in depth what went wrong
and what succeded, you can use this code snippet to inspect the child
errors of the returned error:

```C++
    report.open([=](Error err) {
        if (err) {
            size_t idx = 0;
            for (SharedPtr<Error> child_err: err.child_errors) {
                if (!child_err) {
                    debug("- %d succeeded", idx);
                    continue;
                }
                debug("- %d failed: %s", idx, child_err->explain().c_str());
                idx += 1;
            }
            // Note: given that `open` is idempotent, here you can
            // safely retry by calling `open` again
            return;
        }
        // Operations to be performed on success...
    });
```

# BUGS

You cannot overlap calls to `open`, `write_entry` or `close`. That is, you
MUST NOT call any of the three above method while any of the three above
methods is pending. You MUST first way for the pending method to complete,
which is signalled by the corresponding callback being called.

# HISTORY

The `Report` class appeared in MeasurementKit 0.3.0.
