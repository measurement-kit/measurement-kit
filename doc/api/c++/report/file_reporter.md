# NAME
FileReporter -- Writes test reports on disk

# LIBRARY
MeasurementKit (libmeasurement-kit, -lmeasurement-kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

using namespace measurement_kit::common;
using namespace measurement_kit;

report::FileReporter reporter;

std::string test_name = reporter.test_name;
std::string test_version = reporter.test_version;
std::string probe_ip = reporter.probe_ip;
std::string probe_asn = reporter.probe_asn;
std::string probe_cc = reporter.probe_cc;
time_t start_time = reporter.start_time;
Settings settings = reporter.options;

reporter.on_error([](Error err) {
    // Handle error that otherwise is thrown
});

reporter.emit_error(GenericError());  // Emit a generic error

// The following methods may emit I/O errors
reporter.open()
reporter.write_entry(report::Entry);
reporter.close();

```

# DESCRIPTION

The `FileReporter` object allows to write test reports on disk.

# BUGS

Currently this object emits `GenericError` for any I/O error that
occurs rather than reporting what actually occurred.

# HISTORY

The `FileReporter` class appeared in MeasurementKit 1.0.0.
