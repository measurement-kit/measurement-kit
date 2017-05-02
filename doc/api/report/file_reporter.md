# NAME
file_reporter &mdash; Writes report on a file

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS
```C++
#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

class FileReporter : public BaseReporter {
  public:
    static Var<BaseReporter> make(std::string path);
    Continuation<Error> open(Report) override;
    Continuation<Error> write_entry(Entry e) override;
    Continuation<Error> close() override;
  private:
    BaseReporter() {}
};

} // namespace report
} // namespace mk
```

# STABILITY

2 - Stable

# DESCRIPTION

The `FileReporter` class is a `BaseReporter` that writes the report onto the
file passed as first argument of the `make` factory.

If the file passed as first argument is `-` then the report will be
written on the standard output rather than on a file named `-`.

A report file will consist of zero or more entries. Each entry is a JSON
document serialized on a single line of text. Newlines could either be `\n`
or `\r\n`.

# HISTORY

The `FileReporter` class appeared in MeasurementKit 0.2.0.
