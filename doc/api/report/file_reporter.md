# NAME
file_reporter &mdash; Writes report on a file

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

class FileReporter {
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
    std::string filename;

    Error open();
    Error write_entry(Entry &entry);
    Error close();
};

} // namespace report
} // namespace mk
```

# STABILITY

1 - Experimental

# DESCRIPTION

The `FileReporter` class contains several fields that should be initialized
by the `OoniTest` object owning the reporter. Among such fields, `filename` is
the pathname where the report file will be created. A report file will consist
of zero or more entries. Each entry is a JSON document serialized on a single
line of text. Newlines could either be `\n` or `\r\n`.

The `open()` method opens the report creating the file on disk.

The `write_entry()` method adds extra fields to the current entry (copying from
the `FileReport` class fields) and then writes the entry on the open report file.

The `close()` method closes the report file.

These three methods return `NoError()` in case of success and one of the
following errors in case of failure:

- *mk::report::ReportAlreadyOpen*: if `open()` is called more than once
- *mk::report::ReportNotOpen*: if `write_entry()` or `close()` is called
  but no report was ever `open()`ed
- *mk::report::ReportAlreadyClosed*: if `close()` was called and further
  operations are attempted on the report
- *mk::report::ReportEofError*: if writing the report yields EOF
- *mk::report::ReportIoError*: if writing the report yields a I/O error
- *mk::report::ReportLogicalError*: if it is not possible to convert
  input data and serialize them into the report file

# HISTORY

The `report` module appeared in MeasurementKit 0.2.0.
